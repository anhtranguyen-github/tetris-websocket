#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

// Function prototypes
PGconn* connectToDatabase();
void create_game(PGconn* conn, int host_id, int room_id);
void execute_query(PGconn* conn, const char* query);

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

// Function to execute a query and handle errors
void execute_query(PGconn* conn, const char* query) {
    PGresult* res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Query failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        exit(EXIT_FAILURE);
    }
    PQclear(res);
}

// Function to create a new game
void create_game(PGconn* conn, int host_id, int room_id) {
    // Step 1: Check if the host is valid for the room
    char check_host_query[256];
    snprintf(check_host_query, sizeof(check_host_query),
             "SELECT 1 FROM rooms WHERE room_id = %d AND host_id = %d;", room_id, host_id);

    PGresult* res = PQexec(conn, check_host_query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) == 0) {
        fprintf(stderr, "Error: Host ID %d is not associated with Room ID %d.\n", host_id, room_id);
        PQclear(res);
        return;
    }
    PQclear(res);

    // Step 2: Check if there’s already an active game in the room
    char check_active_game_query[256];
    snprintf(check_active_game_query, sizeof(check_active_game_query),
             "SELECT 1 FROM games WHERE room_id = %d AND status = 1;", room_id);

    res = PQexec(conn, check_active_game_query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK || PQntuples(res) > 0) {
        fprintf(stderr, "Error: There is already an active game in Room ID %d.\n", room_id);
        PQclear(res);
        return;
    }
    PQclear(res);

    // Step 3: Create the game
    char create_game_query[256];
    snprintf(create_game_query, sizeof(create_game_query),
             "INSERT INTO games (room_id, status) VALUES (%d, 1);", room_id);

    execute_query(conn, create_game_query);

    // Step 4: Retrieve the newly created game_id
    char get_game_id_query[256];
    snprintf(get_game_id_query, sizeof(get_game_id_query),
             "SELECT game_id FROM games WHERE room_id = %d AND status = 1 ORDER BY start_time DESC LIMIT 1;", room_id);

    res = PQexec(conn, get_game_id_query);
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        int game_id = atoi(PQgetvalue(res, 0, 0));
        printf("Game created successfully! Game ID: %d\n", game_id);
    } else {
        fprintf(stderr, "Error: Failed to retrieve the new game ID.\n");
    }
    PQclear(res);
}

int main() {
    // Connect to the database
    PGconn* conn = connectToDatabase();

    // Test creating a game with example host_id and room_id
    int host_id = 4;  // Replace with actual host_id
    int room_id = 3;  // Replace with actual room_id

    create_game(conn, host_id, room_id);

    // Close the database connection
    PQfinish(conn);
    return 0;
}
