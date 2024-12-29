#include "../src/server/object.h"
#include "../src/ultis.h"



int main() {
    PGconn *conn = PQconnectdb("dbname=tetris user=new_user password=1 host=localhost");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }
    printf("Connected to database.\n");

    int room_count = 0;
    RoomInfo **rooms = get_all_room_info(conn, &room_count);

    if (rooms) {
        for (int i = 0; i < room_count; i++) {
            printf("Room ID: %d, Room Name: %s\n", rooms[i]->room_id, rooms[i]->room_name);
            free(rooms[i]); // Free each RoomInfo object
        }
        free(rooms); // Free the array of pointers
    } else {
        printf("No rooms found or an error occurred.\n");
    }

    PQfinish(conn);
    return 0;
}