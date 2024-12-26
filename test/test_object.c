#include "../src/server/object.h"
#include "../src/ultis.h"



int main() {
    // Initialize online games
    init_online_games();
    printf("Initialized online game slots.\n");

    // Connect to the PostgreSQL database
    PGconn *conn = PQconnectdb("dbname=tetris user=new_user password=1 host=localhost");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return 1;
    }
    printf("Connected to database.\n");

    // Simulate creating an online game
    int room_id = 1;
    printf("Creating online game for room ID: %d\n", room_id);

    create_online_game(conn, room_id); // Function under test

    printf("Created online game.\n");

    // Find the created game
    int game_slot = -1;
    for (int i = 0; i < MAX_GAME; i++) {
        if (online_game[i].room_id == room_id) {
            game_slot = i;
            break;
        }
    }

    if (game_slot == -1) {
        printf("Online game not found for room ID: %d\n", room_id);
    } else {
        // Serialize the online game
        char *serialized_game = serializeOnlineGame(&online_game[game_slot]);
        if (serialized_game != NULL) {
            printf("Serialized Online Game:\n%s\n", serialized_game);
            free(serialized_game);
        } else {
            printf("Failed to serialize online game.\n");
        }

        // Create a start game message
        Message startGameMessage = create_start_game_message(&online_game[game_slot]);
        
        // Print the message content
        printf("Start Game Message:\n");
        printf("  Type: %d\n", startGameMessage.type);
        printf("  Username: %s\n", startGameMessage.username);
        printf("  Room Name: %s\n", startGameMessage.room_name);
        printf("  Data:\n%s\n", startGameMessage.data);

        int shapeListInt[200];
        get_shape_list_int(shapeListInt, &startGameMessage);

        // Verify the extracted shape list
        printf("Shape list: ");
        for (int i = 0; i < 200; i++) {
            printf("%d ", shapeListInt[i]);
        }
        printf("\n");
        }

    // Cleanup
    PQfinish(conn);  // Close the database connection
    printf("Database connection closed. Test completed.\n");

    return 0;
}
