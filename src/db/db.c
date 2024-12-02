#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

PGconn *connect_db() {
    // Use the default "postgres" database to connect to the server
    const char *conninfo = "dbname=postgres user=new_user password=1 host=localhost";
    
    // Step 1: Connect to PostgreSQL using the "postgres" database
    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database server failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        exit(1);
    }

    // Step 2: Check if the 'tetris' database exists, create it if necessary
    const char *create_db_sql = "SELECT 1 FROM pg_database WHERE datname = 'tetris';";
    PGresult *res = PQexec(conn, create_db_sql);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error checking for database existence: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(1);
    }

    if (PQntuples(res) == 0) {
        // The database doesn't exist, so we create it
        printf("Database 'tetris' does not exist. Creating it...\n");
        const char *create_db_command = "CREATE DATABASE tetris;";
        res = PQexec(conn, create_db_command);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Error creating database: %s\n", PQerrorMessage(conn));
            PQclear(res);
            PQfinish(conn);
            exit(1);
        }

        printf("Database 'tetris' created successfully.\n");
    } else {
        printf("Database 'tetris' already exists.\n");
    }

    // Step 3: Now connect to the 'tetris' database
    PQclear(res);
    const char *final_conninfo = "dbname=tetris user=new_user password=1 host=localhost";
    PGconn *tetris_conn = PQconnectdb(final_conninfo);

    if (PQstatus(tetris_conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to 'tetris' database failed: %s\n", PQerrorMessage(tetris_conn));
        PQfinish(tetris_conn);
        PQfinish(conn);  // Finish the initial connection too
        exit(1);
    }

    printf("Connected to the 'tetris' database.\n");
    PQfinish(conn);  // Close the initial connection since we don't need it anymore
    return tetris_conn;
}


void execute_query(PGconn *conn, const char *query) {
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "SQL error: %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        exit(1);
    }

    printf("Query executed successfully.\n");
    PQclear(res);
}


void create_tables(PGconn *conn) {
    const char *create_users = 
        "CREATE TABLE IF NOT EXISTS users ("
        "user_id SERIAL PRIMARY KEY,"
        "username VARCHAR(50) UNIQUE NOT NULL,"
        "password_hash VARCHAR(255) NOT NULL,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";


    const char *create_sessions = 
        "CREATE TABLE IF NOT EXISTS sessions ("
        "session_id VARCHAR(255) PRIMARY KEY,"
        "username VARCHAR(50) NOT NULL REFERENCES users(username) ON DELETE CASCADE,"
        "expiration TIMESTAMP NOT NULL);";


    const char *create_rooms = 
        "CREATE TABLE IF NOT EXISTS rooms ("
        "room_id SERIAL PRIMARY KEY,"
        "room_name VARCHAR(100) UNIQUE NOT NULL,"
        "host_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,"
        "time_limit INT CHECK (time_limit > 0),"
        "brick_limit INT CHECK (brick_limit > 0),"
        "max_players INT NOT NULL CHECK (max_players BETWEEN 2 AND 8),"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";

    const char *create_room_players = 
        "CREATE TABLE IF NOT EXISTS room_players ("
        "room_id INT NOT NULL REFERENCES rooms(room_id) ON DELETE CASCADE,"
        "user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,"
        "joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "PRIMARY KEY (room_id, user_id));";

    const char *create_games = 
        "CREATE TABLE IF NOT EXISTS games ("
        "game_id SERIAL PRIMARY KEY,"
        "room_id INT NOT NULL REFERENCES rooms(room_id) ON DELETE CASCADE,"
        "start_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
        "end_time TIMESTAMP,"
        "status INT NOT NULL DEFAULT 1 CHECK (status IN (0, 1)));";

    const char *create_game_scores = 
        "CREATE TABLE IF NOT EXISTS game_scores ("
        "game_id INT NOT NULL REFERENCES games(game_id) ON DELETE CASCADE,"
        "user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,"
        "score INT NOT NULL DEFAULT 0,"
        "PRIMARY KEY (game_id, user_id));";


    execute_query(conn, create_users);
    execute_query(conn, create_sessions);
    execute_query(conn, create_rooms);
    execute_query(conn, create_room_players);
    execute_query(conn, create_games);
    execute_query(conn, create_game_scores);
    
    
}


void insert_user(PGconn *conn, const char *username, const char *password_hash) {
    const char *query = 
        "INSERT INTO users (username, password_hash) VALUES ($1, $2) RETURNING user_id;";
    
    const char *paramValues[2] = {username, password_hash};
    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Insert failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("User inserted with ID: %s\n", PQgetvalue(res, 0, 0));
    PQclear(res);
}
void fetch_users(PGconn *conn) {
    const char *query = "SELECT user_id, username, created_at FROM users;";
    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Fetch failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        printf("ID: %s, Username: %s, Created At: %s\n",
               PQgetvalue(res, i, 0),
               PQgetvalue(res, i, 1),
               PQgetvalue(res, i, 2));
    }

    PQclear(res);
}
void close_connection(PGconn *conn) {
    PQfinish(conn);
    printf("Disconnected from database.\n");
}

void create_room(PGconn *conn, const char *room_name, int host_id, int time_limit, int brick_limit, int max_players) {
    const char *query = 
        "INSERT INTO rooms (room_name, host_id, time_limit, brick_limit, max_players) "
        "VALUES ($1, $2, $3, $4, $5) RETURNING room_id;";
    
    const char *paramValues[5];
    char host_id_str[12], time_limit_str[12], brick_limit_str[12], max_players_str[12];
    snprintf(host_id_str, 12, "%d", host_id);
    snprintf(time_limit_str, 12, "%d", time_limit);
    snprintf(brick_limit_str, 12, "%d", brick_limit);
    snprintf(max_players_str, 12, "%d", max_players);

    paramValues[0] = room_name;
    paramValues[1] = host_id_str;
    paramValues[2] = time_limit_str;
    paramValues[3] = brick_limit_str;
    paramValues[4] = max_players_str;

    PGresult *res = PQexecParams(conn, query, 5, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error creating room: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("Room created with ID: %s\n", PQgetvalue(res, 0, 0));
    PQclear(res);
}

void create_game(PGconn *conn, int room_id) {
    const char *query = 
        "INSERT INTO games (room_id, start_time, status) VALUES ($1, CURRENT_TIMESTAMP, 1) RETURNING game_id;";
    
    char room_id_str[12];
    snprintf(room_id_str, 12, "%d", room_id);

    const char *paramValues[1] = {room_id_str};
    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error creating game: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("Game created with ID: %s\n", PQgetvalue(res, 0, 0));
    PQclear(res);
}


void update_game_status(PGconn *conn, int game_id, int status) {
    const char *query = 
        "UPDATE games SET status = $1, end_time = CASE WHEN $1 = 0 THEN CURRENT_TIMESTAMP ELSE NULL END "
        "WHERE game_id = $2;";
    
    char status_str[2], game_id_str[12];
    snprintf(status_str, 2, "%d", status);
    snprintf(game_id_str, 12, "%d", game_id);

    const char *paramValues[2] = {status_str, game_id_str};
    PGresult *res = PQexecParams(conn, query, 2, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        fprintf(stderr, "Error updating game status: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    printf("Game ID %d status updated to %d (%s).\n", game_id, status, status == 0 ? "finished" : "ongoing");
    PQclear(res);
}


int main() {
    PGconn *conn = connect_db();

    create_tables(conn);

    // Adding multiple users
    insert_user(conn, "player1", "hashedpassword1");
    insert_user(conn, "player2", "hashedpassword2");
    insert_user(conn, "player3", "hashedpassword3");
    insert_user(conn, "player4", "hashedpassword4");
    insert_user(conn, "player5", "hashedpassword5");

    // Fetch all users to verify
    fetch_users(conn);

    // Create multiple rooms
    create_room(conn, "Room1", 1, 60, 150, 4);
    create_room(conn, "Room2", 2, 45, 100, 6);
    create_room(conn, "Room3", 3, 30, 200, 5);
    create_room(conn, "Room4", 4, 90, 300, 3);

    // Create a game and update its status
    create_game(conn, 1);
    update_game_status(conn, 1, 0);

    close_connection(conn);
    return 0;
}
