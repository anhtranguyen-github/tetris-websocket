#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "tetris_game.h"

Shape current;

int Table[20][15] = {0};
int score = 0;
int GameOn = 1;
int timer = 400;
int decrease = 1;

LeaderboardEntry leaderboard[LEADERBOARD_SIZE] = {
    {"Player1", 0},
    {"Player2", 0},
    {"Player3", 0},
    {"Player4", 0},
    {"Player5", 0}
};

const int ShapesArray[7][4][4] = {
    // Different Tetris shapes
    {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},    // S shape
    {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // Z shape
    {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // T shape
    {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // L shape
    {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},    // Reverse L shape
    {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},    // Square shape
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}     // Long bar shape
};

void drawBlock(SDL_Renderer *renderer, int x, int y, SDL_Color color) {
    SDL_Rect rect = {x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black border
    SDL_RenderDrawRect(renderer, &rect);
}

Shape copyShape(const int shapeArray[4][4], int width) {
    Shape shape;
    shape.width = width;
    shape.row = 0;
    shape.col = COLS / 2 - width / 2;
    shape.array = malloc(width * sizeof(int*));
    for (int i = 0; i < width; i++) {
        shape.array[i] = malloc(width * sizeof(int));
        for (int j = 0; j < width; j++) {
            shape.array[i][j] = shapeArray[i][j];
        }
    }
    return shape;
}

void freeShape(Shape shape) {
    for (int i = 0; i < shape.width; i++) {
        free(shape.array[i]);
    }
    free(shape.array);
}

int checkPosition(Shape shape) {
    for (int i = 0; i < shape.width; i++) {
        for (int j = 0; j < shape.width; j++) {
            if (shape.array[i][j]) {
                int x = shape.col + j;
                int y = shape.row + i;
                if (x < 0 || x >= COLS || y >= ROWS || (y >= 0 && Table[y][x])) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void newRandomShape() {
    freeShape(current);
    int index = rand() % 7;
    current = copyShape(ShapesArray[index], 4);
    if (!checkPosition(current)) {
        GameOn = 0;
    }
}

void mergeShape() {
    for (int i = 0; i < current.width; i++) {
        for (int j = 0; j < current.width; j++) {
            if (current.array[i][j]) {
                Table[current.row + i][current.col + j] = 1;
            }
        }
    }
}

void clearLines() {
    int cleared = 0;
    for (int i = ROWS - 1; i >= 0; i--) {
        int full = 1;
        for (int j = 0; j < COLS; j++) {
            if (!Table[i][j]) {
                full = 0;
                break;
            }
        }
        if (full) {
            cleared++;
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < COLS; j++) {
                    Table[k][j] = Table[k - 1][j];
                }
            }
            for (int j = 0; j < COLS; j++) {
                Table[0][j] = 0;
            }
            i++;
        }
    }
    score += cleared * 100;
    updatePlayerScore(score);
}

void moveShapeDown() {
    current.row++;
    if (!checkPosition(current)) {
        current.row--;
        mergeShape();
        clearLines();
        newRandomShape();
    }
}

void rotateShape() {
    Shape temp = copyShape((const int (*)[4])current.array, current.width);
    // Set temp's position to match the current shape's position
    temp.row = current.row;
    temp.col = current.col;

    // Rotate the shape's blocks
    for (int i = 0; i < current.width; i++) {
        for (int j = 0; j < current.width; j++) {
            temp.array[i][j] = current.array[current.width - j - 1][i];
        }
    }

    // Check if the rotated shape fits in its new position
    if (checkPosition(temp)) {
        freeShape(current);  // Free memory of the old shape
        current = temp;      // Replace current shape with rotated temp shape
    } else {
        freeShape(temp);     // If rotation is not possible, discard temp
    }
}


void handleEvents(int *quit) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_LEFT: current.col--; if (!checkPosition(current)) current.col++; break;
                case SDLK_RIGHT: current.col++; if (!checkPosition(current)) current.col--; break;
                case SDLK_DOWN: moveShapeDown(); break;
                case SDLK_UP: rotateShape(); break;
            }
        }
    }
}



void renderLeaderboard(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color white = {255, 255, 255};
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;
    char buffer[50];

    // Render each leaderboard entry
    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        snprintf(buffer, sizeof(buffer), "%d. %s: %d", i + 1, leaderboard[i].name, leaderboard[i].score);
        surface = TTF_RenderText_Solid(font, buffer, white);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        
        // Position leaderboard entries to the right of the game screen
        rect.x = SCREEN_WIDTH - 300;  // Assuming SCREEN_WIDTH is the width of the game area
        rect.y = 10 + i * 30;        // Adjust spacing between entries
        rect.w = surface->w;
        rect.h = surface->h;

        SDL_FreeSurface(surface);
        SDL_RenderCopy(renderer, texture, NULL, &rect);
        SDL_DestroyTexture(texture);
    }
}

void renderGame(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Color colors[] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
    
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Render the gameplay area
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (Table[i][j]) {
                drawBlock(renderer, j, i, colors[0]);
            }
        }
    }
    for (int i = 0; i < current.width; i++) {
        for (int j = 0; j < current.width; j++) {
            if (current.array[i][j]) {
                drawBlock(renderer, current.col + j, current.row + i, colors[1]);
            }
        }
    }

    // Draw the border around the gameplay area
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White color for the border
    SDL_Rect gameplayBorder = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    SDL_RenderDrawRect(renderer, &gameplayBorder);

    // Draw the line to separate gameplay and leaderboard
    SDL_RenderDrawLine(renderer, SCREEN_WIDTH - 350, 0, SCREEN_WIDTH - 350, SCREEN_HEIGHT);

    // Render the leaderboard
    renderLeaderboard(renderer, font);

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}

void updatePlayerScore(int newScore) {
    leaderboard[0].score = newScore;
}
