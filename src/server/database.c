#include <stdio.h>
#include <stdlib.h>
#include "database.h"

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