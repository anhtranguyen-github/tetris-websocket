#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <libpq-fe.h>
#include "../protocol/network.h"
#include "../protocol/protocol.h"
#include "../../config/client_config.h"
#include "../ultis.h"
#include <uuid/uuid.h>
#include "database.h"
#include "object.h"

int server_fd;
extern OnlineGame online_game[MAX_GAME];
extern OnlineUser online_users[MAX_USERS];

int find_empty_slot()
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (online_users[i].is_authenticated == 0)
        {
            return i;
        }
    }
    return -1; // No empty slot found
}

void write_to_log_OnlineUser()
{
    write_to_log("Writinggg......................");
    char log_message[8192]; // Large buffer to store all online user info
    char user_entry[512];   // Buffer for each user's details
    strcpy(log_message, "Online Users Log:\n");
    strcat(log_message, "---------------------------------------------------------------------------------------------------------------\n");
    strcat(log_message, "| ID   | Username            | Session ID       | SocketFD  | IP Address    | Port  | Last Activity        | Authenticated | Room ID | Hosting     |\n");
    strcat(log_message, "---------------------------------------------------------------------------------------------------------------\n");

    for (int i = 0; i < MAX_USERS; i++)
    {
        if (online_users[i].is_authenticated)
        {
            // Convert last activity time to readable format
            char time_buffer[26];
            struct tm *tm_info = localtime(&online_users[i].last_activity);
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

            // Get IP address as string
            char ip_address[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(online_users[i].client_addr.sin_addr), ip_address, INET_ADDRSTRLEN);
            int port = ntohs(online_users[i].client_addr.sin_port); // Get port number

            // Format the user entry
            snprintf(user_entry, sizeof(user_entry),
                     "| %-4d | %-20s | %-15s | %-10d | %-15s | %-5d | %-20s | %-13s | %-7d | %-12s |\n",
                     online_users[i].user_id,
                     online_users[i].username,
                     online_users[i].session_id,
                     online_users[i].socket_fd,
                     ip_address,
                     port,
                     time_buffer,
                     online_users[i].is_authenticated ? "YES" : "NO",
                     online_users[i].room_id,
                     online_users[i].is_hosting ? "YES" : "NO");

            strcat(log_message, user_entry);
        }
    }

    strcat(log_message, "---------------------------------------------------------------------------------------------------------------\n");

    // Write the final log message to the log file
    write_to_log(log_message);
}


void add_online_user(int user_id, const char *username, const char *session_id, int socket_fd, struct sockaddr_in client_addr) 
{
    // Check for an existing user with the same username
    for (int i = 0; i < MAX_USERS; i++) 
    {
        if (strcmp(online_users[i].username, username) == 0) 
        {
            // Replace the existing user with the new one
            OnlineUser *user = &online_users[i];
            user->user_id = user_id;
            strncpy(user->username, username, sizeof(user->username) - 1);
            user->username[sizeof(user->username) - 1] = '\0'; // Ensure null termination
            strncpy(user->session_id, session_id, sizeof(user->session_id) - 1);
            user->session_id[sizeof(user->session_id) - 1] = '\0'; // Ensure null termination
            user->socket_fd = socket_fd;
            user->client_addr = client_addr;
            user->last_activity = time(NULL); // Set current time as last activity
            user->is_authenticated = 1;       // User is authenticated
            
            // Reset the room_id for the new session
            user->room_id = -1;

            char log_message[256];
            snprintf(log_message, sizeof(log_message),
                     "Duplicate username found. Replacing old user: [ID: %d, Username: %s, SessionID: %s, IP: %s, SocketFD: %d, RoomID: %d]",
                     user->user_id, user->username, user->session_id, inet_ntoa(client_addr.sin_addr), socket_fd, user->room_id);
            write_to_log(log_message);
            return;
        }
    }

    // Find an empty slot for the new user
    int index = find_empty_slot();
    if (index == -1) 
    {
        write_to_log("No empty slots for online users.");
        return;
    }

    // Add the new user
    OnlineUser *user = &online_users[index];
    user->user_id = user_id;
    strncpy(user->username, username, sizeof(user->username) - 1);
    user->username[sizeof(user->username) - 1] = '\0'; // Ensure null termination
    strncpy(user->session_id, session_id, sizeof(user->session_id) - 1);
    user->session_id[sizeof(user->session_id) - 1] = '\0'; // Ensure null termination
    user->socket_fd = socket_fd;
    user->client_addr = client_addr;
    user->last_activity = time(NULL); // Set current time as last activity
    user->is_authenticated = 1;       // User is authenticated
    user->room_id = -1;               // Initialize room_id to -1 for a new user

    char log_message[256];
    snprintf(log_message, sizeof(log_message),
             "User added to online users: [ID: %d, Username: %s, SessionID: %s, IP: %s, SocketFD: %d, RoomID: %d]",
             user->user_id, user->username, user->session_id, inet_ntoa(client_addr.sin_addr), socket_fd, user->room_id);
    write_to_log(log_message);
}


void generateSessionID(char *sessionID);
// Signal handler to clean up and close the server socket
void handle_signal(int sig)
{
    printf("\nReceived signal %d, shutting down...\n", sig);
    if (server_fd > 0)
    {
        close(server_fd);
    }
    exit(0);
}

// Establish a connection to the PostgreSQL database
PGconn *connectToDatabase()
{
    PGconn *conn = PQconnectdb("dbname=tetris user=new_user password=1 host=localhost");
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Database connection failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        exit(EXIT_FAILURE);
    }
    return conn;
}

// Validate login credentials
bool validateLogin(PGconn *conn, const char *username, const char *password)
{
    const char *query = "SELECT password_hash FROM users WHERE username = $1;";
    const char *paramValues[1] = {username};

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0)
    {
        PQclear(res);
        return false;
    }

    const char *storedPasswordHash = PQgetvalue(res, 0, 0);
    bool valid = strcmp(storedPasswordHash, password) == 0;
    PQclear(res);
    return valid;
}

// Create a new session with an expiration time
bool createSession(PGconn *conn, const char *username, char *sessionID, int expirationMinutes)
{
    // Check if the session ID already exists in the database
    const char *checkQuery = "SELECT 1 FROM sessions WHERE session_id = $1;";
    const char *checkParams[1] = {sessionID};
    PGresult *checkRes = PQexecParams(conn, checkQuery, 1, NULL, checkParams, NULL, NULL, 0);

    if (PQntuples(checkRes) > 0)
    {
        // Session ID already exists, generate a new one and try again
        PQclear(checkRes);
        generateSessionID(sessionID);
        return createSession(conn, username, sessionID, expirationMinutes); // Try again with the new session ID
    }

    PQclear(checkRes);

    // Proceed with inserting the new session
    const char *query = "INSERT INTO sessions (session_id, username, expiration) VALUES ($1, $2, NOW() + INTERVAL '1 minute' * $3)";
    char expirationStr[12];                                                  // Buffer to hold string representation of the integer
    snprintf(expirationStr, sizeof(expirationStr), "%d", expirationMinutes); // Convert integer to string

    const char *paramValues[3] = {sessionID, username, expirationStr};
    PGresult *res = PQexecParams(conn, query, 3, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Session creation failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

// Validate if a session ID is active and not expired
bool validateSession(PGconn *conn, const char *sessionID)
{
    const char *query = "SELECT 1 FROM sessions WHERE session_id = $1 AND expiration > NOW();";
    const char *paramValues[1] = {sessionID};

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);
    bool valid = (PQntuples(res) > 0);
    PQclear(res);
    return valid;
}

void generateSessionID(char *sessionID)
{
    uuid_t uuid;
    uuid_generate(uuid);           // Generate a UUID
    uuid_unparse(uuid, sessionID); // Convert the UUID to a string
}

Message handle_login(const Message *msg, PGconn *conn)
{

    Message response;
    response.type = LOGIN_FAILURE; // Default to failure
    write_to_log("1");
    // Check if the username and password are valid
    if (validateLogin(conn, msg->username, msg->data))
    { // Assuming msg->data contains password
        // Login successful
        response.type = LOGIN_SUCCESS;
        strcpy(response.data, "Login successful!");

        // Generate a session ID (simulating a cookie)
        char sessionID[64];
        generateSessionID(sessionID);

        // Store the session in the database
        if (createSession(conn, msg->username, sessionID, 30))
        {                                     // 30 minutes expiry
            strcpy(response.data, sessionID); // Set session ID as the cookie
        }
        else
        {
            strcpy(response.data, "Failed to create session.");
            response.type = LOGIN_FAILURE;
        }
    }
    else
    {
        // Invalid login
        strcpy(response.data, "Login failed!");
    }
    write_to_log_OnlineUser();
    return response;
}

// Updated handle_login function
Message handle_login2(const Message *msg, PGconn *conn, int socket_fd, struct sockaddr_in client_addr)
{
    write_to_log_OnlineUser();
    Message response;
    response.type = LOGIN_FAILURE; // Default to failure
    write_to_log("1");             // Debug point

    // Check if the username and password are valid
    if (validateLogin(conn, msg->username, msg->data))
    { // Assuming msg->data contains the password
        // Login successful
        response.type = LOGIN_SUCCESS;
        strcpy(response.data, "Login successful!");
        write_to_log("2: Login successful for user");

        // Generate a session ID (simulating a cookie)
        char sessionID[64];
        generateSessionID(sessionID);
        write_to_log("3: Session ID generated");

        // Store the session in the database
        if (createSession(conn, msg->username, sessionID, 30))
        { // 30-minute expiry
            write_to_log("4: Session created in database");

            // Update response with the session ID
            strncpy(response.data, sessionID, sizeof(response.data) - 1);

            // Simulate user_id retrieval (could be from the DB)
            int user_id = 1; // This should come from the users table (getUserID function)

            // Add the user to the `online_users` array
            add_online_user(user_id, msg->username, sessionID, socket_fd, client_addr);
        }
        else
        {
            write_to_log("5: Failed to create session");
            strcpy(response.data, "Failed to create session.");
            response.type = LOGIN_FAILURE;
        }
    }
    else
    {
        // Invalid login
        write_to_log("6: Invalid login attempt");
        strcpy(response.data, "Login failed!");
    }

    return response;
}

int insert_user(PGconn *conn, const char *username, const char *password_hash)
{
    const char *query = "INSERT INTO users (username, password_hash) VALUES ($1, $2);";
    const char *paramValues[2] = {username, password_hash};

    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "Error inserting user: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return 0; // Failure
    }

    PQclear(res);
    return 1; // Success
}

int is_username_taken(PGconn *conn, const char *username)
{
    const char *query = "SELECT COUNT(*) FROM users WHERE username = $1;";
    const char *paramValues[1] = {username};

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error checking username: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1; // Indicate an error
    }

    int count = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return count > 0;
}

void register_user(PGconn *conn, const char *username, const char *password)
{
    // Check if the username already exists
    if (is_username_taken(conn, username))
    {
        printf("The username '%s' is already taken. Please choose a different username.\n", username);
        return;
    }

    // Check if the password length is greater than 6
    if (strlen(password) <= 6)
    {
        printf("Password must be longer than 6 characters.\n");
        return;
    }

    // Hash the password (a placeholder, replace with a real hashing function in production)
    char password_hash[256];
    snprintf(password_hash, sizeof(password_hash), "hashed_%s", password);

    // Insert the user into the database
    insert_user(conn, username, password_hash);
}

Message handle_register(Message *msg, PGconn *conn)
{
    Message response;
    memset(&response, 0, sizeof(Message));

    // Set the response message type to REGISTER by default
    response.type = REGISTER;

    // Validate the username
    if (is_username_taken(conn, msg->username))
    {
        response.type = REGISTER_FAILURE;
        snprintf(response.data, sizeof(response.data), "The username '%s' is already taken.", msg->username);
        return response;
    }

    // Validate the password length
    if (strlen(msg->data) <= 6)
    {
        response.type = REGISTER_FAILURE;
        snprintf(response.data, sizeof(response.data), "Password must be longer than 6 characters.");
        return response;
    }

    // Hash the password (a placeholder hashing mechanism)
    char password_hash[BUFFER_SIZE + 10];
    // pls don't add the "hashed_" before the password :)
    snprintf(password_hash, sizeof(password_hash), "%s", msg->data);

    // Try to register the user in the database
    if (insert_user(conn, msg->username, password_hash))
    {
        response.type = REGISTER_SUCCESS;
        snprintf(response.data, sizeof(response.data), "User '%s' registered successfully.", msg->username);
    }
    else
    {
        response.type = REGISTER_FAILURE;
        snprintf(response.data, sizeof(response.data), "Failed to register the user. Please try again later.");
    }

    return response;
}

Message handle_create_room(Message *msg, PGconn *conn)
{
    Message response;
    memset(&response, 0, sizeof(Message));

    write_to_log("handle_create_room: Starting function execution");

    write_to_log("handle_create_room: Received message");
    write_to_log(msg->room_name);
    write_to_log(msg->data);

    char session_id[MAX_SESSION_ID];
    int time_limit, brick_limit, max_player;
    int parsed = sscanf(msg->data, "%[^|]|%d|%d|%d", session_id, &time_limit, &brick_limit, &max_player);

    write_to_log("Parsed session_id: ");
    write_to_log(session_id);
    write_to_log("Parsed time_limit: ");
    write_to_log_int(time_limit);
    write_to_log("Parsed brick_limit: ");
    write_to_log_int(brick_limit);
    write_to_log("Parsed max_player: ");
    write_to_log_int(max_player);

    if (parsed != 4)
    {
        write_to_log("handle_create_room: Error parsing msg->data. Expected 4 fields but got fewer.");
        time_limit = DEFAULT_TIME_LIMIT;
        brick_limit = DEFAULT_BRICK_LIMIT;
        max_player = DEFAULT_MAX_PLAYER;
    }

    char time_limit_str[64], brick_limit_str[64], max_player_str[64];
    snprintf(time_limit_str, sizeof(time_limit_str), "%d", time_limit);
    snprintf(brick_limit_str, sizeof(brick_limit_str), "%d", brick_limit);
    snprintf(max_player_str, sizeof(max_player_str), "%d", max_player);

    write_to_log("handle_create_room: Converted parameters to strings:");
    write_to_log(time_limit_str);
    write_to_log(brick_limit_str);
    write_to_log(max_player_str);

    const char *query = "INSERT INTO rooms (room_name, host_id, time_limit, brick_limit, max_players) "
                        "VALUES ($1, (SELECT user_id FROM users WHERE username = (SELECT username FROM sessions WHERE session_id = $2)), $3, $4, $5) "
                        "RETURNING room_id;";
    const char *paramValues[5] = {msg->room_name, session_id, time_limit_str, brick_limit_str, max_player_str};

    write_to_log("handle_create_room: Preparing to execute query");

    PGresult *res = PQexecParams(conn, query, 5, NULL, paramValues, NULL, NULL, 0);

    if (res == NULL)
    {
        write_to_log("handle_create_room: PQexecParams returned NULL");
        write_to_log("PostgreSQL error: ");
        write_to_log(PQerrorMessage(conn));
    }
    else
    {
        char log_msg[256];
        snprintf(log_msg, sizeof(log_msg), "handle_create_room: Query executed, status: %s", PQresStatus(PQresultStatus(res)));
        write_to_log(log_msg);

        if (PQresultStatus(res) != PGRES_TUPLES_OK)
        {
            write_to_log("handle_create_room: PostgreSQL query execution error");
            write_to_log(PQresultErrorMessage(res));
        }
    }

    bool success = (res != NULL && PQresultStatus(res) == PGRES_TUPLES_OK);
    int room_id = -1;

    if (success)
    {
        char *room_id_str = PQgetvalue(res, 0, 0);
        room_id = atoi(room_id_str);
        write_to_log("handle_create_room: Room ID retrieved");
        write_to_log(room_id_str);
    }

    PQclear(res);

    if (success)
    {
        write_to_log("handle_create_room: Updating OnlineUser structure");

        for (int i = 0; i < MAX_USERS; i++)
        {
            if (online_users[i].is_authenticated && strcmp(online_users[i].session_id, session_id) == 0)
            {
                online_users[i].room_id = room_id;
                online_users[i].is_hosting = 1;
                write_to_log("handle_create_room: Updated OnlineUser:");
                write_to_log(online_users[i].username);
                break;
            }
        }

        strcpy(response.data, "Room created successfully!");
        response.type = CREATE_ROOM_SUCCESS;
    }
    else
    {
        write_to_log("handle_create_room: Room creation failed");
        strcpy(response.data, "Failed to create room.");
        response.type = CREATE_ROOM_FAILURE;
    }

    write_to_log("handle_create_room: Returning response");
    write_to_log_OnlineUser();
    return response;
}

void broadcast_message_to_room(int room_id, Message *message)
{
    if (!message)
    {
        write_to_log("Error: Message is NULL");
        return;
    }

    char log_buffer[BUFFER_SIZE];
    snprintf(log_buffer, BUFFER_SIZE, "Broadcasting message to room_id: %d", room_id);
    write_to_log(log_buffer);

    for (int i = 0; i < MAX_USERS; i++)
    {
        OnlineUser *user = &online_users[i];

        // Log user details
        snprintf(log_buffer, BUFFER_SIZE, "Checking user %d: username=%s, socket_fd=%d, room_id=%d", i, user->username, user->socket_fd, user->room_id);
        write_to_log(log_buffer);

        // Check if the user is active, authenticated, and belongs to the specified room
        if (user->socket_fd > 0 && user->is_authenticated && user->room_id == room_id)
        {
            snprintf(log_buffer, BUFFER_SIZE, "Sending message to user: %s (socket_fd: %d)", user->username, user->socket_fd);
            write_to_log(log_buffer);
            write_to_log(message->data);
            ssize_t bytes_sent = send(user->socket_fd, message, sizeof(Message), 0);

            if (bytes_sent < 0)
            {
                perror("send");
                snprintf(log_buffer, BUFFER_SIZE, "Failed to send message to user: %s (socket_fd: %d)", user->username, user->socket_fd);
                write_to_log(log_buffer);
            }
            else
            {
                snprintf(log_buffer, BUFFER_SIZE, "Message broadcasted to user: %s (socket_fd: %d)", user->username, user->socket_fd);
                write_to_log(log_buffer);
            }
        }
    }
}

int get_brick_limits(int room_id, PGconn *conn)
{
    if (conn == NULL)
    {
        fprintf(stderr, "Database connection is null.\n");
        return -1; // Return -1 to indicate an error.
    }

    const char *query = "SELECT brick_limit FROM rooms WHERE room_id = $1;";
    PGresult *res = PQexecParams(
        conn,
        query,
        1,               // Number of parameters
        NULL,            // Parameter types (let the server infer types)
        (const char *[]){// Parameter values
                         (const char *)&room_id},
        NULL, // Parameter lengths (not needed for int4)
        NULL, // Parameter formats (text mode)
        0     // Result format (0 = text, 1 = binary)
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Error executing query: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1; // Return -1 to indicate an error.
    }

    if (PQntuples(res) == 0)
    {
        fprintf(stderr, "No room found with room_id %d.\n", room_id);
        PQclear(res);
        return -1; // Return -1 to indicate an error or no result.
    }

    // Parse the brick_limit value.
    int brick_limit = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return brick_limit;
}

Message handle_join_room(Message *msg, PGconn *conn)
{
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

    if (PQntuples(result) == 0)
    {
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
    if (current_players >= max_players)
    {
        write_to_log("Room is full.");
        response.type = ROOM_FULL;
        snprintf(response.data, BUFFER_SIZE, "Room '%s' is full.", room_name);
        return response;
    }

    // Check if the game has already started
    if (game_status == 1)
    {
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

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
        write_to_log("Failed to add player to room.");
        write_to_log(PQerrorMessage(conn)); // Log the detailed error from PostgreSQL

        response.type = JOIN_ROOM_FAILURE;
        snprintf(response.data, BUFFER_SIZE, "Failed to join room '%s'.", room_name);
        PQclear(result);
        return response;
    }

    PQclear(result);
    write_to_log("Player successfully added to room.");

    for (int i = 0; i < MAX_USERS; i++)
    {
        OnlineUser *user = &online_users[i];
        if (user->is_authenticated && strcmp(user->username, msg->username) == 0)
        {
            user->room_id = room_id;
            write_to_log("Updated online_users entry with new room_id:");
            write_to_log_int(user->room_id);
            break;
        }
    }

    // Construct the success response
    response.type = ROOM_JOINED;
    RoomInfo *room_info = get_room_info(conn, room_id);
    // snprintf(response.data, BUFFER_SIZE, "Successfully joined room '%s'.", room_name);
    snprintf(response.data, BUFFER_SIZE,
             "%s|%d|%d|%d|%s",
             room_info->room_name,
             room_info->time_limit,
             room_info->brick_limit,
             room_info->max_players,
             room_info->room_players);

    strncpy(response.room_name, room_name, MAX_ROOM_NAME);
    strncpy(response.username, msg->username, MAX_USERNAME);

    write_to_log("Response constructed successfully:");
    write_to_log(response.data);
    free(room_info);

    Message broadcast_msg = {0};
    broadcast_msg.type = PLAYER_JOINED;
    snprintf(broadcast_msg.data, BUFFER_SIZE, "Player '%s' has joined the room.", msg->username);
    broadcast_message_to_room(room_id, &broadcast_msg);

    return response;
}

Message handle_join_random_room(Message *msg, PGconn *conn)
{
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

    if (PQntuples(result) == 0)
    {
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

    if (PQresultStatus(result) != PGRES_COMMAND_OK)
    {
        write_to_log("Failed to add player to random room.");
        write_to_log(PQerrorMessage(conn));

        response.type = JOIN_ROOM_FAILURE;
        snprintf(response.data, BUFFER_SIZE, "Failed to join room '%s'.", room_name);
        PQclear(result);
        return response;
    }

    PQclear(result);
    write_to_log("Player successfully added to random room.");

    for (int i = 0; i < MAX_USERS; i++)
    {
        OnlineUser *user = &online_users[i];
        if (user->is_authenticated && strcmp(user->username, msg->username) == 0)
        {
            user->room_id = room_id;
            write_to_log("Updated online_users entry with new room_id:");
            write_to_log_int(user->room_id);
            break;
        }
    }

    // Construct success response
    response.type = ROOM_JOINED;

    RoomInfo *room_info = get_room_info(conn, room_id);

    // snprintf(response.data, BUFFER_SIZE, "Successfully joined room '%s'.", room_name);
    snprintf(response.data, BUFFER_SIZE,
             "%s|%d|%d|%d|%s",
             room_info->room_name,
             room_info->time_limit,
             room_info->brick_limit,
             room_info->max_players,
             room_info->room_players);
    strncpy(response.room_name, room_name, MAX_ROOM_NAME);
    strncpy(response.username, msg->username, MAX_USERNAME);

    write_to_log("Response constructed successfully:");
    write_to_log(response.data);
    write_to_log_int(response.type);
    free(room_info);

    Message broadcast_msg = {0};
    broadcast_msg.type = PLAYER_JOINED;
    snprintf(broadcast_msg.data, BUFFER_SIZE, "Player '%s' has joined the room.", msg->username);
    broadcast_message_to_room(room_id, &broadcast_msg);

    return response;
}





Message handle_start_game(Message *msg, PGconn *conn)
{
    Message response;

    // Log the start of the function
    write_to_log("handle_start_game called.");
    write_to_log("Received message:");
    write_to_log(msg->username);


    // Set response defaults
    response.type = START_GAME_FAILURE; // Default to failure
    strncpy(response.username, "system", MAX_USERNAME);
    response.username[MAX_USERNAME - 1] = '\0'; // Ensure null-termination

    // Log the username and default response type
    write_to_log("Default response initialized. Type set to START_GAME_FAILURE.");
    write_to_log("Default username set to 'system'.");

    // Check if the user is hosting the room
    if (!is_user_hosting(msg->username))
    {
        snprintf(response.data, BUFFER_SIZE, "Only the host can start the game.");
        write_to_log("User is not the host. Exiting with message:");
        write_to_log(response.data);
        return response;
    }

    // Log that the user is hosting
    write_to_log("User is the host. Proceeding to get room ID.");

    // Get the room ID associated with the user's username
    int room_id = get_room_id_by_username(msg->username);
    write_to_log("Retrieved room ID:");
    write_to_log_int(room_id);

    if (room_id == -1)
    {
        snprintf(response.data, BUFFER_SIZE, "Room not found for the user.");
        write_to_log("Room ID not found. Exiting with message:");
        write_to_log(response.data);
        return response;
    }

    // Log the creation of the online game
    write_to_log("Room ID found. Creating online game...");
    create_online_game(conn, room_id);

    int game_slot = -1;
    for (int i = 0; i < MAX_GAME; i++)
    {
        if (online_game[i].room_id == room_id)
        {
            game_slot = i;
            break;
        }
    }

    write_to_log("Checked online game slots. Game slot found:");
    write_to_log_int(game_slot);

    if (game_slot == -1)
    {
        snprintf(response.data, BUFFER_SIZE, "Online game not found for room ID: %d", room_id);
        write_to_log("Online game not found. Exiting with message:");
        write_to_log(response.data);
    }
    else
    {
        // Serialize the online game
        char *serialized_game = serializeOnlineGame(&online_game[game_slot]);
        if (serialized_game != NULL)
        {
            write_to_log("Serialized Online Game:");
            write_to_log(serialized_game);
            free(serialized_game);

            // Set response type to success
            response.type = START_GAME_SUCCESS;
            snprintf(response.data, BUFFER_SIZE, "Game started successfully.");
            write_to_log("Game started successfully. Response type set to START_GAME_SUCCESS.");
        }
        else
        {
            write_to_log("Failed to serialize online game.");
        }

        // Create a start game message
        Message startGameMessage = create_start_game_message(&online_game[game_slot]);
        write_to_log("Start game message created successfully.");
        broadcast_message_to_room(room_id, &startGameMessage);
        return response;
    }

    return response;
}

// Periodically clean up expired sessions in the database
void cleanupExpiredSessions(PGconn *conn)
{
    const char *query = "DELETE FROM sessions WHERE expiration < NOW();";
    PGresult *res = PQexecParams(conn, query, 0, NULL, NULL, NULL, NULL, 0);
    PQclear(res);
}

// Process a request from the client
void handleClientRequest(int clientSocket, PGconn *conn)
{
    Message msg, response;

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Get client's IP address
    getpeername(clientSocket, (struct sockaddr *)&client_addr, &addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);

    char log_message[256];
    snprintf(log_message, sizeof(log_message),
             "New client connected from IP: %s, SocketFD: %d",
             client_ip, clientSocket);
    write_to_log(log_message);

    while (recv(clientSocket, &msg, sizeof(Message), 0) > 0)
    {
        snprintf(log_message, sizeof(log_message),
                 "Server received message of type %d from user %s (IP: %s, SocketFD: %d)",
                 msg.type, msg.username, client_ip, clientSocket);
        write_to_log(log_message);

        switch (msg.type)
        {
        case REGISTER:
            response = handle_register(&msg, conn);
            send(clientSocket, &response, sizeof(Message), 0);
            break;
        case LOGIN:
            response = handle_login2(&msg, conn, clientSocket, client_addr);
            // response = handle_login(&msg, conn);  // Handle login and generate session (cookie)
            send(clientSocket, &response, sizeof(Message), 0);
            break;
        case CREATE_ROOM:
            // Call handle_create_room and get the response message
            response = handle_create_room(&msg, conn);
            send(clientSocket, &response, sizeof(Message), 0);
            break;

        case JOIN_ROOM:
            response = handle_join_room(&msg, conn);
            send(clientSocket, &response, sizeof(Message), 0);
            break;

        case JOIN_RANDOM:
            response = handle_join_random_room(&msg, conn);
            send(clientSocket, &response, sizeof(Message), 0);
            break;
        case START_GAME:
            response = handle_start_game(&msg, conn);
            send(clientSocket, &response, sizeof(Message), 0);
            break;

        case DISCONNECT:
            strcpy(response.data, "Disconnected from server.");
            write_to_log("User disconnected");
            response.type = DISCONNECT;
            break;

        default:
            //strcpy(response.data, "Unknown message type.");
            write_to_log("Recieved unknown type");
            write_to_log(response.data);
            response.type = -1;
        }
        write_to_log("====================================");

        // send(clientSocket, &response, sizeof(Message), 0);
    }

    close(clientSocket);
}

void startServer()
{
    server_fd = create_server_socket();
    printf("Server listening on port %d...\n", PORT);
}

// Thread function to handle client connection
void *clientThread(void *arg)
{
    int clientSocket = *((int *)arg);
    free(arg);

    PGconn *conn = connectToDatabase();
    handleClientRequest(clientSocket, conn);
    PQfinish(conn);

    return NULL;
}

// Clean up resources and terminate the server
void stopServer()
{
    if (server_fd > 0)
    {
        close(server_fd);
        server_fd = 0;
    }
    printf("Server stopped.\n");
}

int main()
{
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    init_online_games();
    printf("Initialized online game slots.\n");

    startServer();

    while (1)
    {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        pthread_t tid;
        int *client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        if (pthread_create(&tid, NULL, clientThread, client_fd_ptr) != 0)
        {
            perror("Failed to create thread");
            free(client_fd_ptr);
            close(client_fd); // Close the client socket in case of failure
        }
    }

    stopServer();
    return 0;
}