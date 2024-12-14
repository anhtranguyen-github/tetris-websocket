#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "protocol/protocol.h"

#define LEADERBOARD_SIZE 8
#define BLOCK_SIZE 30
#define COLS 15
#define ROWS 20
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_SHAPES 100
#define MAX_USERNAME 32
#define MAX_PASSWORD 32

extern int Table[20][15];
extern int score;
extern int GameOn;
extern int timer;
extern int decrease;

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

typedef struct {
    SDL_Rect rect;
    SDL_Color color;
    const char *text;
} Button;

extern Shape current;
extern ShapeList shapeList;
extern LeaderboardEntry leaderboard[LEADERBOARD_SIZE];
extern const int ShapesArray[7][4][4];

void initShapeList();
void generateShapes(int number);
void drawBlock(SDL_Renderer *renderer, int x, int y, SDL_Color color);
Shape copyShape(const int shapeArray[4][4], int width);
void freeShape(Shape shape);
int checkPosition(Shape shape);
void newRandomShape();
void newRandomShape2();
void mergeShape();
void clearLines();
void moveShapeDown();
void rotateShape();
void handleEvents(int *quit);
void renderLeaderboard(SDL_Renderer *renderer, TTF_Font *font, const char *roomPlayers);
void renderGame(SDL_Renderer *renderer, TTF_Font *font, const char *roomPlayers);
void updatePlayerScore(int newScore);
void renderButton(SDL_Renderer *renderer, TTF_Font *font, Button button);
int handleButtonClick(Button button, int x, int y);
void renderLoginScreen(SDL_Renderer *renderer, TTF_Font *font, const char *username, const char *password, int usernameSelected);
void handleLoginEvents(int *quit, int *loginSuccess, char *username, char *password, int *usernameSelected, int client_fd);
void renderCreateRoomScreen(SDL_Renderer *renderer, TTF_Font *font, const char *username, const char *room_name, int time_limit, int brick_limit, int max_player, int selectedField);
void handleCreateRoomEvents(int *quit, int *createRoomSuccess, char *username, char *room_name, int *time_limit, int *brick_limit, int *max_player, int *selectedField, int client_fd);
void handleJoinRoomEvents(int *quit, int *joinRoomSuccess, char *username, char *room_name, int client_fd);
void handleJoinRandomRoomEvents(int *quit, int *joinRoomSuccess, char *username, int client_fd, Message *response);
void renderWaitingRoom(SDL_Renderer *renderer, TTF_Font *font, const char *room_name, int time_limit, int brick_limit, int max_players, const char *room_players);
void handleWaitingRoomEvents(int *quit, int *startGame);

#endif // TETRIS_GAME_H
