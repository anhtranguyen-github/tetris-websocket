#include <stdio.h>
#include <stdlib.h>
#include <libpq-fe.h>

int get_brick_limits(int room_id, PGconn *conn) {
    if (conn == NULL) {
        fprintf(stderr, "Database connection is null.\n");
        return -1;
    }

    const char *query = "SELECT brick_limit FROM rooms WHERE room_id = $1;";
    const char *param_values[1];
    char room_id_str[10];
    snprintf(room_id_str, sizeof(room_id_str), "%d", room_id);
    param_values[0] = room_id_str;

    PGresult *res = PQexecParams(
        conn,
        query,
        1,
        NULL,
        param_values,
        NULL,
        NULL,
        0
    );

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Error executing query: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return -1;
    }

    if (PQntuples(res) == 0) {
        fprintf(stderr, "No room found with room_id %d.\n", room_id);
        PQclear(res);
        return -1;
    }

    int brick_limit = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);
    return brick_limit;
}

int main() {
    const char *conninfo = "dbname=tetris user=new_user password=1 host=localhost";
    
    // Step 1: Connect to PostgreSQL
    PGconn *conn = PQconnectdb(conninfo);
    
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }

    int room_id = 1; // Test with room_id = 1.
    int brick_limit = get_brick_limits(room_id, conn);

    if (brick_limit != -1) {
        printf("The brick limit for room %d is %d.\n", room_id, brick_limit);
    } else {
        printf("Failed to retrieve brick limit for room %d.\n", room_id);
    }

    PQfinish(conn);
    return 0;
}
