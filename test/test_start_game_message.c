#include "../src/server/object.h"
#include "../src/ultis.h"
#include <string.h>
#include <stdio.h>
#include <time.h>


// Mock constants and data for testing
#define MAX_TEST_BUFFER 1024

// Mock function to initialize rooms
void init_mock_rooms() {
    for (int i = 0; i < MAX_ROOMS; i++) {
        room_infor[i].room_id = i + 1;
        snprintf(room_infor[i].room_name, sizeof(room_infor[i].room_name), "Room_%d", i + 1);
        snprintf(room_infor[i].room_players, sizeof(room_infor[i].room_players), "PlayerA,PlayerB,PlayerC");
        room_infor[i].brick_limit = 10;
    }
}

// Test for generate_random_game_id
void test_generate_random_game_id() {
    printf("Testing generate_random_game_id...\n");
    int id1 = generate_random_game_id();
    int id2 = generate_random_game_id();
    printf("Generated IDs: %d, %d\n", id1, id2);
}

// Test for init_leaderboard
void test_init_leaderboard() {
    printf("\nTesting init_leaderboard...\n");
    LeaderboardEntry leaderboard[LEADERBOARD_SIZE];
    init_leaderboard(leaderboard, "Alice,Bob,Charlie", LEADERBOARD_SIZE);
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        printf("Rank %d: %s - %d\n", i + 1, leaderboard[i].name, leaderboard[i].score);
    }
}

// Test for add_online_games
void test_add_online_games() {
    printf("\nTesting add_online_games...\n");
    add_online_games(1); // Add game for room ID 1
    for (int i = 0; i < MAX_GAME; i++) {
        if (online_game[i].game_id != 0) {
            printf("Game added - Room ID: %d, Game ID: %d\n", online_game[i].room_id, online_game[i].game_id);
        }
    }
}

// Test for update_leaderboard
void test_update_leaderboard() {
    printf("\nTesting update_leaderboard...\n");
    add_online_games(1); // Ensure a game exists
    int game_id = online_game[0].game_id;
    printf("Updating leaderboard for game ID: %d\n", game_id);
    update_leaderboard(game_id, "PlayerA", 10);
    update_leaderboard(game_id, "PlayerB", 20);
    update_leaderboard(game_id, "Unknown", 15);
}

// Test for serialize_shape_list and parse_shape_list
void test_serialize_and_parse_shape_list() {
    printf("\nTesting serialize_shape_list and parse_shape_list...\n");
    ShapeList shape_list;
    char buffer[MAX_TEST_BUFFER] = {0};

    init_random_shapelist(&shape_list, 5);
    printf("Serializing ShapeList...\n");
    int bytes_written = serialize_shape_list(&shape_list, buffer, MAX_TEST_BUFFER);
    printf("Serialized Data (%d bytes):\n%s\n", bytes_written, buffer);

    ShapeList parsed_list;
    parse_shape_list(buffer, &parsed_list);
    printf("Parsed ShapeList has %d shapes.\n", parsed_list.count);
}

// Test for serialize_leaderboard and parse_leaderboard
void test_serialize_and_parse_leaderboard() {
    printf("\nTesting serialize_leaderboard and parse_leaderboard...\n");
    LeaderboardEntry leaderboard[LEADERBOARD_SIZE];
    char buffer[MAX_TEST_BUFFER] = {0};

    init_leaderboard(leaderboard, "Alice,Bob,Charlie", LEADERBOARD_SIZE);
    printf("Serializing Leaderboard...\n");
    int bytes_written = serialize_leaderboard(leaderboard, buffer, MAX_TEST_BUFFER);
    printf("Serialized Data (%d bytes):\n%s\n", bytes_written, buffer);

    LeaderboardEntry parsed_leaderboard[LEADERBOARD_SIZE];
    parse_leaderboard(buffer, parsed_leaderboard);
    printf("Parsed Leaderboard:\n");
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        printf("Rank %d: %s - %d\n", i + 1, parsed_leaderboard[i].name, parsed_leaderboard[i].score);
    }
}

// Test for get_start_game_message
void test_get_start_game_message() {
    printf("\nTesting get_start_game_message...\n");
    add_online_games(1); // Ensure a game exists
    int game_id = online_game[0].game_id;
    Message msg = get_start_game_message(game_id);

    printf("Generated START_GAME message:\n%s\n", msg.data);
}

int main() {
    printf("=== Starting Tests ===\n");
    init_mock_rooms(); // Initialize mock rooms for testing

    test_generate_random_game_id();
    test_init_leaderboard();
    test_add_online_games();
    test_update_leaderboard();
    test_serialize_and_parse_shape_list();
    test_serialize_and_parse_leaderboard();
    test_get_start_game_message();

    printf("\n=== All Tests Completed Successfully ===\n");
    return 0;
}