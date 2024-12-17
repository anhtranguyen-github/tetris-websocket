#include "../src/server/object.h"
#include <string.h>
#include <stdio.h>
#include <time.h>


// Test the functions
int main() {
    // Initialize a game
    room_infor[0].room_id = 1;
    strcpy(room_infor[0].room_name, "Room 1");
    room_infor[0].brick_limit = 10;
    strcpy(room_infor[0].room_players, "Alice,Bob,Charlie");

    add_online_games(1);

    // Update leaderboard scores
    int game_id = online_game[0].game_id;

    update_leaderboard(game_id, "Alice", 10);
    update_leaderboard(game_id, "Bob", 15);
    update_leaderboard(game_id, "Charlie", 5);

    // Attempt to update a non-existing player
    update_leaderboard(game_id, "UnknownPlayer", 10);

    return 0;
}
