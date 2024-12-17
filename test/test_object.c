#include "../src/server/object.h"
#include <string.h>
#include <stdio.h>
#include <time.h>


// Test the functions
int main() {
    printf("\n--- Testing Online Game System ---\n");

    // Test: Adding online games
    add_online_games(101);
    add_online_games(102);

    // Test: Updating leaderboard for the first game
    printf("\n--- Updating Leaderboard for Game 1 ---\n");
    update_leaderboard(1, "Alice", 1500);
    update_leaderboard(1, "Bob", 1800);
    update_leaderboard(1, "Charlie", 1700);
    update_leaderboard(1, "Dave", 1600);

    // Test: Display leaderboard
    printf("\n--- Leaderboard for Game 1 ---\n");
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        if (online_game[0].leaderboard[i].score > 0) {
            printf("%d. %s - %d\n", i + 1, online_game[0].leaderboard[i].name, online_game[0].leaderboard[i].score);
        }
    }

    return 0;
}
