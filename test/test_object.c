#include "../src/server/object.h"
#include "../src/ultis.h"



int main() {
    // Initialize online games
    init_online_games();

    // Simulate the rest of the logic
    PGconn *conn = PQconnectdb("dbname=tetris user=new_user password=1 host=localhost");
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("Connection to database failed.\n");
        PQfinish(conn);
        return 1;
    }

    int room_id = 1; 
    create_online_game(conn, room_id);

    room_id = 2;
    create_online_game(conn, room_id);

    room_id = 3; // Test for room_id = 1
    create_online_game(conn, room_id);

    PQfinish(conn);  // Close the database connection
    return 0;
}