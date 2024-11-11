// src/tetris_game.h

#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

// src/tetris_game.c or src/tetris_game.h

#define LEADERBOARD_SIZE 5      // Size of the leaderboard
#define BLOCK_SIZE 30          // Size of each block in the Tetris grid
#define COLS 15                // Number of columns in the grid
#define ROWS 20                // Number of rows in the grid
#define SCREEN_WIDTH 800       // Screen width for rendering
#define SCREEN_HEIGHT 600      // Screen height for rendering


extern int Table[20][15];      // No initialization here
extern int score;
extern int GameOn;
extern int timer;
extern int decrease;

typedef struct {
    int **array;
    int width, row, col;
} Shape;

typedef struct {
    char name[20];
    int score;
} LeaderboardEntry;

extern Shape current;

extern LeaderboardEntry leaderboard[5];
extern const int ShapesArray[7][4][4];

void drawBlock(SDL_Renderer *renderer, int x, int y, SDL_Color color);
Shape copyShape(const int shapeArray[4][4], int width);
void freeShape(Shape shape);
int checkPosition(Shape shape);
void newRandomShape();
void mergeShape();
void clearLines();
void moveShapeDown();
void rotateShape();
void handleEvents(int *quit);
void renderLeaderboard(SDL_Renderer *renderer, TTF_Font *font);
void renderGame(SDL_Renderer *renderer, TTF_Font *font);
void updatePlayerScore(int newScore);

#endif // TETRIS_GAME_H
