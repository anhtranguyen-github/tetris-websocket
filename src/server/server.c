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
#include "../ultis.h"  
#include <uuid/uuid.h>  

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
    write_to_log(msg->data);      // Assuming `data` (session ID) is null-terminated

    // Define the SQL query to insert a new room, using the user_id for host_id
    const char* query = "INSERT INTO rooms (room_name, host_id) "
                        "VALUES ($1, (SELECT user_id FROM users WHERE username = (SELECT username FROM sessions WHERE session_id = $2)));";
    const char* paramValues[2] = { msg->room_name, msg->data };  // sessionID is passed as msg->data

    // Log the preparation for query execution
    write_to_log("handle_create_room: Preparing to execute query");

    // Execute the query with parameters
    PGresult* res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);
    
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
                if (handleJoinRoom(conn, msg.data, msg.room_name)) {
                    strcpy(response.data, "Room joined successfully!");
                    response.type = ROOM_JOINED;
                } else {
                    strcpy(response.data, "Failed to join room. Room may not exist.");
                    response.type = ROOM_NOT_FOUND;
                }
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