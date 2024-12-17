// object.h

#ifndef OBJECT_H
#define OBJECT_H

#include <stdlib.h>
#include <time.h>
#define LEADERBOARD_SIZE 8
#define BLOCK_SIZE 30
#define COLS 15
#define ROWS 20
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_SHAPES 1000
#define MAX_USERNAME 32
#define MAX_PASSWORD 32
#define MAX_CLIENTS 10
#define MAX_ROOMS 5
#define MAX_ROOM_NAME 32
#define BUFFER_SIZE 1000
#define ROOM_PLAYER_BUFFER_SIZE 200
#define MAX_SESSION_ID 40
#define MAX_USERS 100
#define MAX_GAME 100



typedef struct {
    int **array;
    int width, row, col;
} Shape;

typedef struct {
    Shape shapes[MAX_SHAPES];
    int count;
    int current;
} ShapeList;

typedef struct {
    char name[20];
    int score;
} LeaderboardEntry;

typedef struct RoomInfo{
    int room_id;
    char room_name[MAX_ROOM_NAME];
    int time_limit;
    int brick_limit;
    int max_players;
    int current_players;
    char room_players[ROOM_PLAYER_BUFFER_SIZE];
} RoomInfo;

RoomInfo room_infor[MAX_ROOMS];

typedef struct OnlineGame{
    int room_id;
    int game_id;
    ShapeList shape_list;
    LeaderboardEntry leaderboard[LEADERBOARD_SIZE];
} OnlineGame;


OnlineGame online_game[MAX_GAME];

void init_random_shapelist(ShapeList *list, int numbers);
void add_online_games(int room_id);
void update_leaderboard(int game_id, const char *player_name, int score);


#endif // OBJECT_H