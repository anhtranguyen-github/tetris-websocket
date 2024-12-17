// object.c

#include "object.h"
#include <string.h>
#include <stdio.h>


const int ShapesArray[7][4][4] = {
    {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},    // S shape
    {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // Z shape
    {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // T shape
    {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // L shape
    {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // Reverse L shape
    {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},    // Square shape
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}     // Long bar shape
};



void init_random_shapelist(ShapeList *list, int numbers) {
    if (numbers > MAX_SHAPES) numbers = MAX_SHAPES; // Limit shapes to MAX_SHAPES
    list->count = numbers;
    list->current = 0;
    srand(time(NULL)); // Seed the random number generator

    for (int i = 0; i < numbers; i++) {
        int random_shape = rand() % 7; // Select a random shape (0 to 6)
        list->shapes[i].width = 4;
        list->shapes[i].row = ROWS;
        list->shapes[i].col = COLS;

        // Allocate memory for shape array
        list->shapes[i].array = (int **)malloc(4 * sizeof(int *));
        for (int j = 0; j < 4; j++) {
            list->shapes[i].array[j] = (int *)malloc(4 * sizeof(int));
            for (int k = 0; k < 4; k++) {
                list->shapes[i].array[j][k] = ShapesArray[random_shape][j][k];
            }
        }
    }
}

// Add an online game to the list
void add_online_games(int room_id) {
    for (int i = 0; i < MAX_GAME; i++) {
        if (online_game[i].room_id == 0) { // Find an empty slot
            online_game[i].room_id = room_id;
            online_game[i].game_id = i + 1; // Assign a game ID
            init_random_shapelist(&online_game[i].shape_list, 50); // Initialize with 50 random shapes

            // Initialize leaderboard with empty entries
            for (int j = 0; j < LEADERBOARD_SIZE; j++) {
                strcpy(online_game[i].leaderboard[j].name, "");
                online_game[i].leaderboard[j].score = 0;
            }

            printf("Online game added: Room ID = %d, Game ID = %d\n", room_id, online_game[i].game_id);
            return;
        }
    }
    printf("No available slots for a new game.\n");
}

// Update the leaderboard for a specific game
void update_leaderboard(int game_id, const char *player_name, int score) {
    if (game_id < 1 || game_id > MAX_GAME) {
        printf("Invalid Game ID.\n");
        return;
    }

    OnlineGame *game = &online_game[game_id - 1];

    // Check if this game exists
    if (game->room_id == 0) {
        printf("Game ID %d does not exist.\n", game_id);
        return;
    }

    // Insert the new score into the leaderboard
    LeaderboardEntry new_entry;
    strncpy(new_entry.name, player_name, sizeof(new_entry.name) - 1);
    new_entry.name[sizeof(new_entry.name) - 1] = '\0'; // Ensure null-termination
    new_entry.score = score;

    // Find the correct position to insert and shift the rest
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        if (score > game->leaderboard[i].score) {
            // Shift lower scores down
            for (int j = LEADERBOARD_SIZE - 1; j > i; j--) {
                game->leaderboard[j] = game->leaderboard[j - 1];
            }
            game->leaderboard[i] = new_entry;
            printf("Leaderboard updated: Player %s, Score %d\n", player_name, score);
            return;
        }
    }
    printf("Player %s did not make it into the leaderboard.\n", player_name);
}
