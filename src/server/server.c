#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <libpq-fe.h>
#include "../protocol/network.c"
#include "../protocol/protocol.h"
#include "../../config/client_config.h"
#include "../ultis.h"  
#include <uuid/uuid.h>  
#include "database.h"

int server_fd;
void generateSessionID(char *sessionID);  
// Signal handler to clean up and close the server socket
void handle_signal(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    if (server_fd > 0) {
        close(server_fd);
    }
    exit(0);
}


// Establish a connection to the PostgreSQL database
PGconn* connectToDatabase() {
    PGconn* conn = PQconnectdb("dbname=tetris user=new_user password=1 host=localhost");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Database connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        exit(EXIT_FAILURE);
    }
    return conn;
}

// Validate login credentials
bool validateLogin(PGconn* conn, const char* username, const char* password) {
    const char* query = "SELECT password_hash FROM users WHERE username = $1;";
    const char* paramValues[1] = { username };

    PGresult* res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        PQclear(res);
        return false;
    }

    const char* storedPasswordHash = PQgetvalue(res, 0, 0);
    bool valid = strcmp(storedPasswordHash, password) == 0;
    PQclear(res);
    return valid;
}

// Create a new session with an expiration time
bool createSession(PGconn* conn, const char* username, char* sessionID, int expirationMinutes) {
    // Check if the session ID already exists in the database
    const char* checkQuery = "SELECT 1 FROM sessions WHERE session_id = $1;";
    const char* checkParams[1] = { sessionID };
    PGresult* checkRes = PQexecParams(conn, checkQuery, 1, NULL, checkParams, NULL, NULL, 0);

    if (PQntuples(checkRes) > 0) {
        // Session ID already exists, generate a new one and try again
        PQclear(checkRes);
        generateSessionID(sessionID);
        return createSession(conn, username, sessionID, expirationMinutes);  // Try again with the new session ID
    }

    PQclear(checkRes);

    // Proceed with inserting the new session
    const char* query = "INSERT INTO sessions (session_id, username, expiration) VALUES ($1, $2, NOW() + INTERVAL '1 minute' * $3)";
    char expirationStr[12]; // Buffer to hold string representation of the integer
    snprintf(expirationStr, sizeof(expirationStr), "%d", expirationMinutes); // Convert integer to string

    const char* paramValues[3] = { sessionID, username, expirationStr };
    PGresult* res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Session creation failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}


// Validate if a session ID is active and not expired
bool validateSession(PGconn* conn, const char* sessionID) {
    const char* query = "SELECT 1 FROM sessions WHERE session_id = $1 AND expiration > NOW();";
    const char* paramValues[1] = { sessionID };

    PGresult* res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    bool valid = (PQntuples(res) > 0);
    PQclear(res);
    return valid;
}

void generateSessionID(char *sessionID) {
    uuid_t uuid;
    uuid_generate(uuid);  // Generate a UUID
    uuid_unparse(uuid, sessionID);  // Convert the UUID to a string
}

Message handle_login(const Message *msg, PGconn* conn) {
    Message response;
    response.type = LOGIN_FAILURE;  // Default to failure
    write_to_log("1");
    // Check if the username and password are valid
    if (validateLogin(conn, msg->username, msg->data)) {  // Assuming msg->data contains password
        // Login successful
        response.type = LOGIN_SUCCESS;
        strcpy(response.data, "Login successful!");

        // Generate a session ID (simulating a cookie)
        char sessionID[64];
        generateSessionID(sessionID);
        
        // Store the session in the database
        if (createSession(conn, msg->username, sessionID, 30)) {  // 30 minutes expiry
            strcpy(response.data, sessionID);  // Set session ID as the cookie
        } else {
            strcpy(response.data, "Failed to create session.");
            response.type = LOGIN_FAILURE;
        }
    } else {
        // Invalid login
        strcpy(response.data, "Login failed!");
    }

    return response;
}



// Function to handle CREATE_ROOM message
Message handle_create_room(Message* msg, PGconn* conn) {
    Message response;
    memset(&response, 0, sizeof(Message));

    // Log the start of function execution
    write_to_log("handle_create_room: Starting function execution");

    // Log incoming message details
    write_to_log("handle_create_room: Received message");
    write_to_log(msg->room_name); // Assuming `room_name` is null-terminated
    write_to_log(msg->data);      // Assuming `data` contains session ID and room settings (time_limit, brick_limit, max_player)

    // Parse the data from msg->data (session ID | time_limit | brick_limit | max_player)
    char session_id[MAX_SESSION_ID];
    int time_limit, brick_limit, max_player;
    int parsed = sscanf(msg->data, "%[^|]|%d|%d|%d", session_id, &time_limit, &brick_limit, &max_player);

    // Log the parsed values
    write_to_log("Parsed session_id: ");
    write_to_log(session_id);
    write_to_log("Parsed time_limit: ");
    write_to_log_int(time_limit);
    write_to_log("Parsed brick_limit: ");
    write_to_log_int(brick_limit);
    write_to_log("Parsed max_player: ");
    write_to_log_int(max_player);

    // Check if parsing was successful
    if (parsed != 4) {
        write_to_log("handle_create_room: Error parsing msg->data. Expected 4 fields but got fewer.");
        // Set default values if parsing fails
        time_limit = DEFAULT_TIME_LIMIT;
        brick_limit = DEFAULT_BRICK_LIMIT;
        max_player = DEFAULT_MAX_PLAYER;
    }

    // Convert the integers to strings for SQL query
    char time_limit_str[64], brick_limit_str[64], max_player_str[64];
    snprintf(time_limit_str, sizeof(time_limit_str), "%d", time_limit);
    snprintf(brick_limit_str, sizeof(brick_limit_str), "%d", brick_limit);
    snprintf(max_player_str, sizeof(max_player_str), "%d", max_player);

    // Log the converted values
    write_to_log("handle_create_room: Converted parameters to strings:");
    write_to_log(time_limit_str);
    write_to_log(brick_limit_str);
    write_to_log(max_player_str);

    // Define the SQL query to insert a new room, including the new parameters
    const char* query = "INSERT INTO rooms (room_name, host_id, time_limit, brick_limit, max_players) "
                        "VALUES ($1, (SELECT user_id FROM users WHERE username = (SELECT username FROM sessions WHERE session_id = $2)), $3, $4, $5);";
    const char* paramValues[5] = { msg->room_name, session_id, time_limit_str, brick_limit_str, max_player_str };  // Session ID, time_limit, brick_limit, max_players

    // Log the preparation for query execution
    write_to_log("handle_create_room: Preparing to execute query");

    // Execute the query with parameters
    PGresult* res = PQexecParams(conn, query, 5, NULL, paramValues, NULL, NULL, 0);

    // Check if the query execution returned NULL (failure)
    if (res == NULL) {
        write_to_log("handle_create_room: PQexecParams returned NULL");
        // Log the PostgreSQL error message
        write_to_log("PostgreSQL error: ");
        write_to_log(PQerrorMessage(conn)); // Log connection-level error
    } else {
        // Log the result status of the query
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "handle_create_room: Query executed, status: %s",
                 PQresStatus(PQresultStatus(res)));
        write_to_log(log_msg);

        // If the query failed, log the specific PostgreSQL error message
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            write_to_log("handle_create_room: PostgreSQL query execution error");
            write_to_log(PQresultErrorMessage(res));  // Log the query-level error message
        }
    }

    // Determine success based on the query result status
    bool success = (res != NULL && PQresultStatus(res) == PGRES_COMMAND_OK);

    // Clear the result memory
    PQclear(res);

    // Log the result of the operation and set the response type
    if (success) {
        write_to_log("handle_create_room: Room creation successful");
        strcpy(response.data, "Room created successfully!");
        response.type = CREATE_ROOM_SUCCESS;  // Success type
    } else {
        write_to_log("handle_create_room: Room creation failed");
        strcpy(response.data, "Failed to create room.");
        response.type = CREATE_ROOM_FAILURE;  // Failure type
    }

    // Log the final response preparation
    write_to_log("handle_create_room: Returning response");

    // Return the Message struct with the appropriate response
    return response;
}


// Handle joining an existing room
bool handleJoinRoom(PGconn* conn, const char* sessionID, const char* roomID) {
    const char* query = "INSERT INTO room_participants (room_id, user_id) "
                        "VALUES ($1, (SELECT username FROM sessions WHERE session_id = $2));";
    const char* paramValues[2] = { roomID, sessionID };

    PGresult* res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if (!success) {
        fprintf(stderr, "Failed to join room: %s\n", PQerrorMessage(conn));
    }
    PQclear(res);
    return success;
}
Message handle_join_room(Message *msg, PGconn *conn) {
    Message response = {0};
    char session_id[MAX_SESSION_ID], room_name[MAX_ROOM_NAME];
    char query[BUFFER_SIZE];
    PGresult *result;

    write_to_log("Parsing message data to extract session_id and room_name.");
    sscanf(msg->data, "%[^|]|%s", session_id, room_name);
    write_to_log("Parsed session_id:");
    write_to_log(session_id);
    write_to_log("Parsed room_name:");
    write_to_log(room_name);

    // Check if the room exists
    snprintf(query, BUFFER_SIZE,
             "SELECT room_id, max_players, "
             "(SELECT COUNT(*) FROM room_players WHERE room_id = rooms.room_id) AS current_players, "
             "(SELECT status FROM games WHERE room_id = rooms.room_id ORDER BY start_time DESC LIMIT 1) AS game_status "
             "FROM rooms WHERE room_name = '%s';",
             room_name);
    write_to_log("Executing query to check if room exists:");
    write_to_log(query);
    result = PQexec(conn, query);

    if (PQntuples(result) == 0) {
        write_to_log("Room not found.");
        response.type = ROOM_NOT_FOUND;
        snprintf(response.data, BUFFER_SIZE, "Room '%s' not found.", room_name);
        PQclear(result);
        return response;
    }

    int room_id = atoi(PQgetvalue(result, 0, 0));
    int max_players = atoi(PQgetvalue(result, 0, 1));
    int current_players = atoi(PQgetvalue(result, 0, 2));
    int game_status = PQgetvalue(result, 0, 3) ? atoi(PQgetvalue(result, 0, 3)) : 0;

    write_to_log("Room details fetched:");
    write_to_log_int(room_id);
    write_to_log_int(max_players);
    write_to_log_int(current_players);
    write_to_log_int(game_status);

    PQclear(result);

    // Check if the room is full
    if (current_players >= max_players) {
        write_to_log("Room is full.");
        response.type = ROOM_FULL;
        snprintf(response.data, BUFFER_SIZE, "Room '%s' is full.", room_name);
        return response;
    }

    // Check if the game has already started
    if (game_status == 1) {
        write_to_log("Game in the room has already started.");
        response.type = GAME_ALREADY_STARTED;
        snprintf(response.data, BUFFER_SIZE, "Game in room '%s' has already started.", room_name);
        return response;
    }

    // Add the player to the room (fixing the session_id issue)
    snprintf(query, BUFFER_SIZE,
             "INSERT INTO room_players (room_id, user_id) "
             "SELECT %d, u.user_id FROM users u "
             "JOIN sessions s ON u.username = s.username "
             "WHERE s.session_id = '%s';",
             room_id, session_id);
    write_to_log("Executing query to add player to room:");
    write_to_log(query);
    result = PQexec(conn, query);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        write_to_log("Failed to add player to room.");
        write_to_log(PQerrorMessage(conn));  // Log the detailed error from PostgreSQL

        response.type = JOIN_ROOM_FAILURE;
        snprintf(response.data, BUFFER_SIZE, "Failed to join room '%s'.", room_name);
        PQclear(result);
        return response;
    }

    PQclear(result);
    write_to_log("Player successfully added to room.");

    // Construct the success response
    response.type = ROOM_JOINED;
    snprintf(response.data, BUFFER_SIZE, "Successfully joined room '%s'.", room_name);
    strncpy(response.room_name, room_name, MAX_ROOM_NAME);
    strncpy(response.username, msg->username, MAX_USERNAME);

    write_to_log("Response constructed successfully:");
    write_to_log(response.data);

    return response;
}

Message handle_join_random_room(Message *msg, PGconn *conn) {
    Message response = {0};
    char session_id[MAX_SESSION_ID];
    char query[BUFFER_SIZE];
    PGresult *result;

    strncpy(session_id, msg->data, MAX_SESSION_ID);
    write_to_log("Joining a random room for session:");
    write_to_log(session_id);

    // Query to find a random available room
    snprintf(query, BUFFER_SIZE,
             "SELECT room_id, room_name, max_players, "
             "(SELECT COUNT(*) FROM room_players WHERE room_id = rooms.room_id) AS current_players, "
             "(SELECT status FROM games WHERE room_id = rooms.room_id ORDER BY start_time DESC LIMIT 1) AS game_status "
             "FROM rooms "
             "WHERE max_players > (SELECT COUNT(*) FROM room_players WHERE room_id = rooms.room_id) "
             "AND NOT EXISTS (SELECT 1 FROM games WHERE room_id = rooms.room_id AND status = 1) "
             "ORDER BY RANDOM() LIMIT 1;");
    write_to_log("Executing query to find a random room:");
    write_to_log(query);
    result = PQexec(conn, query);

    if (PQntuples(result) == 0) {
        write_to_log("No available random rooms found.");
        response.type = ROOM_NOT_FOUND;
        snprintf(response.data, BUFFER_SIZE, "No available rooms to join.");
        PQclear(result);
        return response;
    }

    int room_id = atoi(PQgetvalue(result, 0, 0));
    char room_name[MAX_ROOM_NAME];
    strncpy(room_name, PQgetvalue(result, 0, 1), MAX_ROOM_NAME);
    int max_players = atoi(PQgetvalue(result, 0, 2));
    int current_players = atoi(PQgetvalue(result, 0, 3));
    int game_status = PQgetvalue(result, 0, 4) ? atoi(PQgetvalue(result, 0, 4)) : 0;

    write_to_log("Random room details fetched:");
    write_to_log_int(room_id);
    write_to_log(room_name);
    write_to_log_int(max_players);
    write_to_log_int(current_players);
    write_to_log_int(game_status);

    PQclear(result);

    // Add the player to the random room
    snprintf(query, BUFFER_SIZE,
             "INSERT INTO room_players (room_id, user_id) "
             "SELECT %d, u.user_id FROM users u "
             "JOIN sessions s ON u.username = s.username "
             "WHERE s.session_id = '%s';",
             room_id, session_id);
    write_to_log("Executing query to add player to random room:");
    write_to_log(query);
    result = PQexec(conn, query);

    if (PQresultStatus(result) != PGRES_COMMAND_OK) {
        write_to_log("Failed to add player to random room.");
        write_to_log(PQerrorMessage(conn));

        response.type = JOIN_ROOM_FAILURE;
        snprintf(response.data, BUFFER_SIZE, "Failed to join room '%s'.", room_name);
        PQclear(result);
        return response;
    }

    PQclear(result);
    write_to_log("Player successfully added to random room.");

    // Construct success response
    response.type = ROOM_JOINED;
    snprintf(response.data, BUFFER_SIZE, "Successfully joined room '%s'.", room_name);
    strncpy(response.room_name, room_name, MAX_ROOM_NAME);
    strncpy(response.username, msg->username, MAX_USERNAME);

    write_to_log("Response constructed successfully:");
    write_to_log(response.data);

    return response;
}



// Periodically clean up expired sessions in the database
void cleanupExpiredSessions(PGconn* conn) {
    const char* query = "DELETE FROM sessions WHERE expiration < NOW();";
    PGresult* res = PQexecParams(conn, query, 0, NULL, NULL, NULL, NULL, 0);
    PQclear(res);
}

// Process a request from the client
void handleClientRequest(int clientSocket, PGconn* conn) {
    Message msg, response;

    while (recv(clientSocket, &msg, sizeof(Message), 0) > 0) {
        printf("Server received message of type %d from %s\n", msg.type, msg.username);

        switch (msg.type) {
            case LOGIN:
                response = handle_login(&msg, conn);  // Handle login and generate session (cookie)
                break;
            case CREATE_ROOM:
                // Call handle_create_room and get the response message
                response = handle_create_room(&msg, conn);
                break;

            case JOIN_ROOM:
                response = handle_join_room(&msg, conn);
                break;
            
            case JOIN_RANDOM:
                response = handle_join_random_room(&msg, conn);
                break;

            case DISCONNECT:
                strcpy(response.data, "Disconnected from server.");
                response.type = DISCONNECT;
                break;

            default:
                strcpy(response.data, "Unknown message type.");
                response.type = -1;
        }

        send(clientSocket, &response, sizeof(Message), 0);
    }

    close(clientSocket);
}


void startServer() {
    server_fd = create_server_socket();
    printf("Server listening on port %d...\n", PORT);
}

// Thread function to handle client connection
void* clientThread(void* arg) {
    int clientSocket = *((int*)arg);
    free(arg);

    PGconn* conn = connectToDatabase();
    handleClientRequest(clientSocket, conn);
    PQfinish(conn);

    return NULL;
}

// Clean up resources and terminate the server
void stopServer() {
    if (server_fd > 0) {
        close(server_fd);
        server_fd = 0;
    }
    printf("Server stopped.\n");
}

int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    startServer();

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t tid;
        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        if (pthread_create(&tid, NULL, clientThread, client_fd_ptr) != 0) {
            perror("Failed to create thread");
            free(client_fd_ptr);
            close(client_fd);  // Close the client socket in case of failure
        }
    }

    stopServer();
    return 0;
}