// object.h

#ifndef OBJECT_H
#define OBJECT_H

#include <stdlib.h>
#include <time.h>
#include "../protocol/protocol.h"
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <libpq-fe.h>
#include <arpa/inet.h>
#include "../protocol/protocol.h"
#include "../../config/client_config.h"
#include <uuid/uuid.h>  
#include "database.h"
#include "object.h"

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
#define BUFFER_SIZE 3000
#define ROOM_PLAYER_BUFFER_SIZE 200
#define MAX_SESSION_ID 40
#define MAX_USERS 100
#define MAX_GAME 100




typedef struct OnlineUser {
    int user_id;                          // Unique ID of the user (from database)
    char username[MAX_USERNAME];     // Username (max length 50)
    char session_id[MAX_SESSION_ID]; // Session ID (max length 255)
    int socket_fd;                       // File descriptor for the user's socket
    struct sockaddr_in client_addr;     // Address information for the user
    time_t last_activity;               // Timestamp of the last activity (for timeouts)
    int is_authenticated;            // Boolean to track if the user is authenticated (1 = yes, 0 = no)
    char ip_address[100];  
    int port;
    char time_buffer[100];
    int room_id;                      
    int is_hosting;                     
} OnlineUser;


extern const int ShapesArray[7][4][4];



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
    char name[MAX_USERNAME];
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

typedef struct RoomPlayerList {
    char **player_names;
    int count;
} RoomPlayerList;



typedef struct Leaderboard{
    LeaderboardEntry entries[LEADERBOARD_SIZE];
    int current_players; // Number of active players in the leaderboard
} Leaderboard;


typedef struct OnlineGame{
    int room_id;    
    int game_id;
    int *shapeList;
    int brick_limit;
    Leaderboard leaderboard;
} OnlineGame;
// Declare variables as extern

extern OnlineGame online_game[MAX_GAME];
extern OnlineUser online_users[MAX_USERS];



//Room infor function   
int generate_random_game_id();
int is_user_hosting(const char *username);

RoomInfo *get_room_info(PGconn *conn, int room_id); 
RoomInfo **get_all_room_info(PGconn *conn, int *count);
int get_brick_limit(RoomInfo *room_info);  
int get_current_players(RoomInfo *room_info);  
RoomPlayerList *get_room_players(RoomInfo *room_info);  
void free_room_player_list(RoomPlayerList *player_list);
int get_room_id_by_username(const char* username);


//shape list
Shape copyShape(const int shapeArray[4][4], int width);  
void freeShape(Shape shape);  
void generateShapes(ShapeList *shapeList, int number);  
void freeShapeList(ShapeList *shapeList);  
char* serializeShapeList(ShapeList *shapeList);  
void deserializeShapeList(ShapeList *shapeList, const char *data);  



//leaderboard 

void initLeaderboard(Leaderboard *leaderboard, RoomPlayerList *player_list);
void update_leaderboard(Leaderboard *leaderboard, const char *player, int points);
char* serializeLeaderboard(const Leaderboard *leaderboard);
void deserializeLeaderboard(Leaderboard *leaderboard, const char *data);

//online game
void init_online_games();
int find_empty_game_slot();
void create_online_game(PGconn *conn, int room_id);


char* serializeOnlineGame(const OnlineGame *game); 

//Message
Message create_start_game_message(const OnlineGame *game);
void get_shape_list_int(int* shapeList, const Message* message);




#endif // OBJECT_H