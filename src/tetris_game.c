#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "tetris_game.h"
#include "client_utils.h"
#include "ultis.h"

Shape current;
ShapeList shapeList;

int Table[20][15] = {0};
int score = 0;
int GameOn = 1;
int timer = 400;
int decrease = 1;

char session_id[MAX_SESSION_ID] = "";

LeaderboardEntry leaderboard[LEADERBOARD_SIZE] = {
    {"", 0},
    {"", 0},
    {"", 0},
    {"", 0},
    {"", 0},
    {"", 0},
    {"", 0},
    {"", 0}
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

void initShapeList() {
    shapeList.count = 0;
    shapeList.current = -1;
}
void generateShapes(int number) {
    for (int i = 0; i < number; i++) {
        int index = rand() % 7;
        Shape newShape = copyShape(ShapesArray[index], 4);
        if (shapeList.count < MAX_SHAPES) {
            shapeList.shapes[shapeList.count++] = newShape;
        } else {
            freeShape(newShape); // Avoid memory leak if list is full
        }
    }
}

Shape viewNextShape(const ShapeList *shapeList) {
    if (shapeList->count == 0) {
        fprintf(stderr, "ShapeList is empty!\n");
        Shape emptyShape = {NULL, 0, 0, 0};
        return emptyShape; // Return an empty shape in case of an error
    }

    // Calculate the index of the next shape
    int nextIndex = (shapeList->current + 1) % shapeList->count;

    // Return the next shape
    return shapeList->shapes[nextIndex];
}

Shape* getNextShape() {
    if (shapeList.count == 0) {
        return NULL; // No shapes available
    }
    shapeList.current = (shapeList.current + 1) % shapeList.count;
    return &shapeList.shapes[shapeList.current];
}
Shape getNextShape2() {
    if (shapeList.count == 0) {
        // Return an empty shape or a default value if no shapes are available
        return (Shape){0}; // Assuming Shape is a struct, this will return an empty/zero-initialized shape
    }
    //printShapeToFile(current, "shape.txt");
    shapeList.current = (shapeList.current + 1) % shapeList.count;
    return shapeList.shapes[shapeList.current];
}


void freeShapeList() {
    for (int i = 0; i < shapeList.count; i++) {
        freeShape(shapeList.shapes[i]);
    }
    shapeList.count = 0;
    shapeList.current = -1;
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
    if (shape.array != NULL) {
        for (int i = 0; i < shape.width; i++) {
            free(shape.array[i]);
        }
        free(shape.array);
        shape.array = NULL;
    }
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

void newRandomShape2() {
    freeShape(current);
    current = getNextShape2();
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

int moveShapeDown() {
    current.row++;
    if (!checkPosition(current)) {
        current.row--;
        mergeShape();
        clearLines();
        newRandomShape2();
        return 1;
    }
    return 0;
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



void drawBlock(SDL_Renderer *renderer, int x, int y, SDL_Color color) {
    SDL_Rect rect = {x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black border
    SDL_RenderDrawRect(renderer, &rect);
}

void renderButton(SDL_Renderer *renderer, TTF_Font *font, Button button) {
    SDL_SetRenderDrawColor(renderer, button.color.r, button.color.g, button.color.b, 255);
    SDL_RenderFillRect(renderer, &button.rect);

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, button.text, textColor);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int textWidth = surface->w;
    int textHeight = surface->h;
    SDL_Rect textRect = {button.rect.x + (button.rect.w - textWidth) / 2, button.rect.y + (button.rect.h - textHeight) / 2, textWidth, textHeight};

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_DestroyTexture(texture);
}

int handleButtonClick(Button button, int x, int y) {
    return (x >= button.rect.x && x <= button.rect.x + button.rect.w && y >= button.rect.y && y <= button.rect.y + button.rect.h);
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

void renderLeaderboard(SDL_Renderer *renderer, TTF_Font *font, const char *roomPlayers) {
    SDL_Color white = {255, 255, 255};
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;
    char buffer[50];

    for (int i = 0; i < LEADERBOARD_SIZE; i++) {
        strncpy(leaderboard[i].name, "", sizeof(leaderboard[i].name));
    }

    char playersCopy[ROOM_PLAYER_BUFFER_SIZE];
    strncpy(playersCopy, roomPlayers, sizeof(playersCopy));
    playersCopy[sizeof(playersCopy) - 1] = '\0';  // Ensure null-terminated string
    char *token = strtok(playersCopy, ",");
    int index = 0;

    while (token != NULL && index < LEADERBOARD_SIZE) {
        strncpy(leaderboard[index].name, token, sizeof(leaderboard[index].name));
        leaderboard[index].name[sizeof(leaderboard[index].name) - 1] = '\0';  // Ensure null-termination
        token = strtok(NULL, ",");
        index++;
    }

    // Render each leaderboard entry
    for (int i = 0; i < index; i++) {
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

    // Render the next shape in the leaderboard area
    Shape nextShape = viewNextShape(&shapeList);

    if (nextShape.array == NULL) {
        printf("Warning: nextShape.array is NULL.\n");
        return;
    }

    SDL_Color shapeColor = {255, 255, 0}; // Yellow color for the next shape
    int shapeStartX = SCREEN_WIDTH - 250; // Position to render next shape
    int shapeStartY = 450;

    for (int i = 0; i < nextShape.width; i++) {
        for (int j = 0; j < nextShape.width; j++) {
            if (nextShape.array[i][j]) {
                int x = shapeStartX + j * BLOCK_SIZE;
                int y = shapeStartY + i * BLOCK_SIZE;

                if (x >= 0 && y >= 0 && x < SCREEN_WIDTH && y < SCREEN_HEIGHT) {
                    drawBlock(renderer, x / BLOCK_SIZE, y / BLOCK_SIZE, shapeColor);
                } else {
                    printf("Warning: Block position out of bounds (x=%d, y=%d)\n", x, y);
                }
            }
        }
    }
}

void renderGame(SDL_Renderer *renderer, TTF_Font *font, const char *roomPlayers) {
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
    renderLeaderboard(renderer, font, roomPlayers);

    // Present the rendered frame
    SDL_RenderPresent(renderer);
}

// TODO: Update the score to other player
void updatePlayerScore(int newScore) {
    leaderboard[0].score = newScore;
}

//Declare Register button Globally
SDL_Rect registerRect;

void renderLoginScreen(SDL_Renderer *renderer, TTF_Font *font, const char *username, const char *password, int usernameSelected) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {128, 128, 128, 255};

    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;

    // Render username label
    surface = TTF_RenderText_Solid(font, "Username:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 - 100;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render username input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect usernameRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 70, 200, 30};
    SDL_RenderDrawRect(renderer, &usernameRect);
    const char *usernameText = strlen(username) > 0 ? username : "Enter username";
    SDL_Color usernameColor = strlen(username) > 0 ? (usernameSelected ? white : gray) : gray;
    surface = TTF_RenderText_Solid(font, usernameText, usernameColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = usernameRect.x + 5;
    rect.y = usernameRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render password label
    surface = TTF_RenderText_Solid(font, "Password:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 - 30;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render password input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect passwordRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 200, 30};
    SDL_RenderDrawRect(renderer, &passwordRect);
    const char *passwordText = strlen(password) > 0 ? password : "Enter password";
    SDL_Color passwordColor = strlen(password) > 0 ? (!usernameSelected ? white : gray) : gray;
    char passwordDisplay[MAX_PASSWORD];
    if (strlen(password) > 0) {
        memset(passwordDisplay, '*', strlen(password));
        passwordDisplay[strlen(password)] = '\0';
    } else {
        strcpy(passwordDisplay, passwordText);
    }
    surface = TTF_RenderText_Solid(font, passwordDisplay, passwordColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = passwordRect.x + 5;
    rect.y = passwordRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render register button
    surface = TTF_RenderText_Solid(font, "Register", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    registerRect.x = SCREEN_WIDTH / 2 - 100;
    registerRect.y = SCREEN_HEIGHT / 2 + 50;
    registerRect.w = 200;
    registerRect.h = 30;

    SDL_RenderDrawRect(renderer, &registerRect);
    rect.x = registerRect.x + 5;
    rect.y = registerRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

void renderRegisterScreen(SDL_Renderer *renderer, TTF_Font *font, const char *username, const char *password, const char *confirmPassword, int selectedField) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {128, 128, 128, 255};

    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;

    // Render username label
    surface = TTF_RenderText_Solid(font, "Username:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 - 100;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render username input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect usernameRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 70, 200, 30};
    SDL_RenderDrawRect(renderer, &usernameRect);
    const char *usernameText = strlen(username) > 0 ? username : "Enter username";
    SDL_Color usernameColor = strlen(username) > 0 ? (selectedField == 0 ? white : gray) : gray;
    surface = TTF_RenderText_Solid(font, usernameText, usernameColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = usernameRect.x + 5;
    rect.y = usernameRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render password label
    surface = TTF_RenderText_Solid(font, "Password:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 - 30;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render password input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect passwordRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 200, 30};
    SDL_RenderDrawRect(renderer, &passwordRect);
    const char *passwordText = strlen(password) > 0 ? password : "Enter password";
    SDL_Color passwordColor = strlen(password) > 0 ? (selectedField == 1 ? white : gray) : gray;
    char passwordDisplay[MAX_PASSWORD];
    if (strlen(password) > 0) {
        memset(passwordDisplay, '*', strlen(password));
        passwordDisplay[strlen(password)] = '\0';
    } else {
        strcpy(passwordDisplay, passwordText);
    }
    surface = TTF_RenderText_Solid(font, passwordDisplay, passwordColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = passwordRect.x + 5;
    rect.y = passwordRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render confirm password label
    surface = TTF_RenderText_Solid(font, "Confirm Password:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 + 40;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render confirm password input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect confirmPasswordRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 70, 200, 30};
    SDL_RenderDrawRect(renderer, &confirmPasswordRect);
    const char *confirmPasswordText = strlen(confirmPassword) > 0 ? confirmPassword : "Confirm password";
    SDL_Color confirmPasswordColor = strlen(confirmPassword) > 0 ? (selectedField == 2 ? white : gray) : gray;
    char confirmPasswordDisplay[MAX_PASSWORD];
    if (strlen(confirmPassword) > 0) {
        memset(confirmPasswordDisplay, '*', strlen(confirmPassword));
        confirmPasswordDisplay[strlen(confirmPassword)] = '\0';
    } else {
        strcpy(confirmPasswordDisplay, confirmPasswordText);
    }
    surface = TTF_RenderText_Solid(font, confirmPasswordDisplay, confirmPasswordColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = confirmPasswordRect.x + 5;
    rect.y = confirmPasswordRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

void handleRegisterEvents(int *quit, int *registerSuccess, char *username, char *password, char *confirmPassword, int *selectedField, int client_fd) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            SDL_Keycode key = e.key.keysym.sym;
            int shiftPressed = (SDL_GetModState() & KMOD_SHIFT);
            if (key == SDLK_TAB) {
                *selectedField = (*selectedField + 1) % 3;
            } else if (key == SDLK_BACKSPACE) {
                if (*selectedField == 0 && strlen(username) > 0) {
                    username[strlen(username) - 1] = '\0';
                } else if (*selectedField == 1 && strlen(password) > 0) {
                    password[strlen(password) - 1] = '\0';
                } else if (*selectedField == 2 && strlen(confirmPassword) > 0) {
                    confirmPassword[strlen(confirmPassword) - 1] = '\0';
                }
            } else if (key == SDLK_RETURN) {
                if (strcmp(password, confirmPassword) == 0) {
                    if (register_user(client_fd, username, password)) {
                        *registerSuccess = 1;
                        printf("Register success.\n");
                    } else {
                        printf("Register failed.\n");
                    }
                } else {
                    printf("Passwords do not match.\n");
                }
            } else if (key >= SDLK_SPACE && key <= SDLK_z) {
                char keyChar = (char)key;
                if (shiftPressed) {
                    // Map shifted characters
                    switch (key) {
                        case SDLK_1: keyChar = '!'; break;
                        case SDLK_2: keyChar = '@'; break;
                        case SDLK_3: keyChar = '#'; break;
                        case SDLK_4: keyChar = '$'; break;
                        case SDLK_5: keyChar = '%'; break;
                        case SDLK_6: keyChar = '^'; break;
                        case SDLK_7: keyChar = '&'; break;
                        case SDLK_8: keyChar = '*'; break;
                        case SDLK_9: keyChar = '('; break;
                        case SDLK_0: keyChar = ')'; break;
                        case SDLK_MINUS: keyChar = '_'; break;
                        case SDLK_EQUALS: keyChar = '+'; break;
                        case SDLK_LEFTBRACKET: keyChar = '{'; break;
                        case SDLK_RIGHTBRACKET: keyChar = '}'; break;
                        case SDLK_BACKSLASH: keyChar = '|'; break;
                        case SDLK_SEMICOLON: keyChar = ':'; break;
                        case SDLK_QUOTE: keyChar = '"'; break;
                        case SDLK_COMMA: keyChar = '<'; break;
                        case SDLK_PERIOD: keyChar = '>'; break;
                        case SDLK_SLASH: keyChar = '?'; break;
                        default:
                            if (key >= SDLK_a && key <= SDLK_z) {
                                keyChar = (char)(key - 32); // Convert to uppercase
                            }
                            break;
                    }
                }
                if (*selectedField == 0 && strlen(username) < MAX_USERNAME - 1) {
                    strncat(username, &keyChar, 1);
                    printf("Username: %s\n", username);
                } else if (*selectedField == 1 && strlen(password) < MAX_PASSWORD - 1) {
                    strncat(password, &keyChar, 1);
                    printf("Password: %s\n", password);
                } else if (*selectedField == 2 && strlen(confirmPassword) < MAX_PASSWORD - 1) {
                    strncat(confirmPassword, &keyChar, 1);
                    printf("Confirm Password: %s\n", confirmPassword);
                }
            }
        }
    }
}

void handleLoginEvents(int *quit, int *loginSuccess, int *showRegisterScreen, char *username, char *password, int *usernameSelected, int client_fd) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = 1;
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);

                    if (x >= registerRect.x && x <= (registerRect.x + registerRect.w) &&
                        y >= registerRect.y && y <= (registerRect.y + registerRect.h)) {
                        printf("Register button clicked!\n");
                        *showRegisterScreen = 1;
                        username[0] = '\0';
                        password[0] = '\0';
                        return;
                    }
        } else if (e.type == SDL_KEYDOWN) {
            SDL_Keycode key = e.key.keysym.sym;
            int shiftPressed = (SDL_GetModState() & KMOD_SHIFT);
            if (key == SDLK_TAB) {
                *usernameSelected = !*usernameSelected;
                printf("Switched input field. Username selected: %d\n", *usernameSelected);
            } else if (key == SDLK_BACKSPACE) {
                if (*usernameSelected && strlen(username) > 0) {
                    username[strlen(username) - 1] = '\0';
                    printf("Username: %s\n", username);
                } else if (!*usernameSelected && strlen(password) > 0) {
                    password[strlen(password) - 1] = '\0';
                    printf("Password: %s\n", password);
                }
            } else if (key == SDLK_RETURN) {
                if (send_login_request(client_fd, username, password, session_id)) {
                    *loginSuccess = 1;
                    printf("Login success.\n");
                } else {
                    printf("Login failed. Invalid username or password.\n");
                }
            } else if (key >= SDLK_SPACE && key <= SDLK_z) {
                char keyChar = (char)key;
                if (shiftPressed) {
                    // Map shifted characters
                    switch (key) {
                        case SDLK_1: keyChar = '!'; break;
                        case SDLK_2: keyChar = '@'; break;
                        case SDLK_3: keyChar = '#'; break;
                        case SDLK_4: keyChar = '$'; break;
                        case SDLK_5: keyChar = '%'; break;
                        case SDLK_6: keyChar = '^'; break;
                        case SDLK_7: keyChar = '&'; break;
                        case SDLK_8: keyChar = '*'; break;
                        case SDLK_9: keyChar = '('; break;
                        case SDLK_0: keyChar = ')'; break;
                        case SDLK_MINUS: keyChar = '_'; break;
                        case SDLK_EQUALS: keyChar = '+'; break;
                        case SDLK_LEFTBRACKET: keyChar = '{'; break;
                        case SDLK_RIGHTBRACKET: keyChar = '}'; break;
                        case SDLK_BACKSLASH: keyChar = '|'; break;
                        case SDLK_SEMICOLON: keyChar = ':'; break;
                        case SDLK_QUOTE: keyChar = '"'; break;
                        case SDLK_COMMA: keyChar = '<'; break;
                        case SDLK_PERIOD: keyChar = '>'; break;
                        case SDLK_SLASH: keyChar = '?'; break;
                        default:
                            if (key >= SDLK_a && key <= SDLK_z) {
                                keyChar = (char)(key - 32); // Convert to uppercase
                            }
                            break;
                    }
                }
                if (*usernameSelected && strlen(username) < MAX_USERNAME - 1) {
                    strncat(username, &keyChar, 1);
                    printf("Username: %s\n", username);
                } else if (!*usernameSelected && strlen(password) < MAX_PASSWORD - 1) {
                    strncat(password, &keyChar, 1);
                    printf("Password: %s\n", password);
                }
            }
        }
    }
}

void renderCreateRoomScreen(SDL_Renderer *renderer, TTF_Font *font, const char *username, const char *room_name, int time_limit, int brick_limit, int max_player, int selectedField) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color gray = {128, 128, 128, 255};

    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;

    // Render username label
    surface = TTF_RenderText_Solid(font, "Username:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 - 150;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render username input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect usernameRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 120, 200, 30};
    SDL_RenderDrawRect(renderer, &usernameRect);
    const char *usernameText = strlen(username) > 0 ? username : "Enter username";
    SDL_Color usernameColor = strlen(username) > 0 ? (selectedField == 0 ? white : gray) : gray;
    surface = TTF_RenderText_Solid(font, usernameText, usernameColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = usernameRect.x + 5;
    rect.y = usernameRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render room name label
    surface = TTF_RenderText_Solid(font, "Room Name:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 - 90;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render room name input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect roomNameRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 60, 200, 30};
    SDL_RenderDrawRect(renderer, &roomNameRect);
    const char *roomNameText = strlen(room_name) > 0 ? room_name : "Enter room name";
    SDL_Color roomNameColor = strlen(room_name) > 0 ? (selectedField == 1 ? white : gray) : gray;
    surface = TTF_RenderText_Solid(font, roomNameText, roomNameColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = roomNameRect.x + 5;
    rect.y = roomNameRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render time limit label
    surface = TTF_RenderText_Solid(font, "Time Limit:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 - 30;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render time limit input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect timeLimitRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 200, 30};
    SDL_RenderDrawRect(renderer, &timeLimitRect);
    char timeLimitText[12];
    snprintf(timeLimitText, sizeof(timeLimitText), "%d", time_limit);
    SDL_Color timeLimitColor = selectedField == 2 ? white : gray;
    surface = TTF_RenderText_Solid(font, timeLimitText, timeLimitColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = timeLimitRect.x + 5;
    rect.y = timeLimitRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render brick limit label
    surface = TTF_RenderText_Solid(font, "Brick Limit:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 + 30;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render brick limit input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect brickLimitRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 60, 200, 30};
    SDL_RenderDrawRect(renderer, &brickLimitRect);
    char brickLimitText[12];
    snprintf(brickLimitText, sizeof(brickLimitText), "%d", brick_limit);
    SDL_Color brickLimitColor = selectedField == 3 ? white : gray;
    surface = TTF_RenderText_Solid(font, brickLimitText, brickLimitColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = brickLimitRect.x + 5;
    rect.y = brickLimitRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render max player label
    surface = TTF_RenderText_Solid(font, "Max Player:", white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = SCREEN_WIDTH / 2 - 100;
    rect.y = SCREEN_HEIGHT / 2 + 90;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render max player input box
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect maxPlayerRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 120, 200, 30};
    SDL_RenderDrawRect(renderer, &maxPlayerRect);
    char maxPlayerText[12];
    snprintf(maxPlayerText, sizeof(maxPlayerText), "%d", max_player);
    SDL_Color maxPlayerColor = selectedField == 4 ? white : gray;
    surface = TTF_RenderText_Solid(font, maxPlayerText, maxPlayerColor);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = maxPlayerRect.x + 5;
    rect.y = maxPlayerRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

void handleCreateRoomEvents(int *quit, char *username, char *room_name, int *time_limit, int *brick_limit, int *max_player, int *selectedField, int client_fd) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            SDL_Keycode key = e.key.keysym.sym;
            int shiftPressed = (SDL_GetModState() & KMOD_SHIFT);
            if (key == SDLK_TAB) {
                *selectedField = (*selectedField + 1) % 6;
            } else if (key == SDLK_BACKSPACE) {
                if (*selectedField == 0 && strlen(username) > 0) {
                    username[strlen(username) - 1] = '\0';
                } else if (*selectedField == 1 && strlen(room_name) > 0) {
                    room_name[strlen(room_name) - 1] = '\0';
                } else if (*selectedField == 2 && *time_limit != TIME_LIMIT_UNLIMITED) {
                    *time_limit = *time_limit / 10;
                } else if (*selectedField == 3 && *brick_limit != BRICK_LIMIT_UNLIMITED) {
                    *brick_limit = *brick_limit / 10;
                } else if (*selectedField == 4 && *max_player > 0) {
                    *max_player = *max_player / 10;
                }
            } else if (key == SDLK_RETURN) {
               create_room(client_fd, username, room_name, session_id, *time_limit, *brick_limit, *max_player);

            } else if (key >= SDLK_SPACE && key <= SDLK_z) {
                char keyChar = (char)key;
                if (shiftPressed) {
                    // Map shifted characters
                    switch (key) {
                        case SDLK_1: keyChar = '!'; break;
                        case SDLK_2: keyChar = '@'; break;
                        case SDLK_3: keyChar = '#'; break;
                        case SDLK_4: keyChar = '$'; break;
                        case SDLK_5: keyChar = '%'; break;
                        case SDLK_6: keyChar = '^'; break;
                        case SDLK_7: keyChar = '&'; break;
                        case SDLK_8: keyChar = '*'; break;
                        case SDLK_9: keyChar = '('; break;
                        case SDLK_0: keyChar = ')'; break;
                        case SDLK_MINUS: keyChar = '_'; break;
                        case SDLK_EQUALS: keyChar = '+'; break;
                        case SDLK_LEFTBRACKET: keyChar = '{'; break;
                        case SDLK_RIGHTBRACKET: keyChar = '}'; break;
                        case SDLK_BACKSLASH: keyChar = '|'; break;
                        case SDLK_SEMICOLON: keyChar = ':'; break;
                        case SDLK_QUOTE: keyChar = '"'; break;
                        case SDLK_COMMA: keyChar = '<'; break;
                        case SDLK_PERIOD: keyChar = '>'; break;
                        case SDLK_SLASH: keyChar = '?'; break;
                        default:
                            if (key >= SDLK_a && key <= SDLK_z) {
                                keyChar = (char)(key - 32); // Convert to uppercase
                            }
                            break;
                    }
                }
                if (*selectedField == 0 && strlen(username) < MAX_USERNAME - 1) {
                    strncat(username, &keyChar, 1);
                } else if (*selectedField == 1 && strlen(room_name) < MAX_ROOM_NAME - 1) {
                    strncat(room_name, &keyChar, 1);
                } else if (*selectedField == 2 && *time_limit < 1000000) {
                    *time_limit = *time_limit * 10 + (key - SDLK_0);
                } else if (*selectedField == 3 && *brick_limit < 1000000) {
                    *brick_limit = *brick_limit * 10 + (key - SDLK_0);
                } else if (*selectedField == 4 && *max_player < 10) {
                    *max_player = *max_player * 10 + (key - SDLK_0);
                }
            }
        }
    }
}

void handleJoinRoomEvents(int *quit, char *username, char *room_name, int client_fd) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            SDL_Keycode key = e.key.keysym.sym;
            int shiftPressed = (SDL_GetModState() & KMOD_SHIFT);
            if (key == SDLK_RETURN) {
                printf("Attempting to join room: %s\n", room_name);
                join_room(client_fd, username, room_name, session_id);
            } else if ((key >= SDLK_SPACE && key <= SDLK_z) && strlen(room_name) < MAX_ROOM_NAME - 1) {
                char keyChar = (char)key;
                if (shiftPressed) {
                    // Map shifted characters
                    switch (key) {
                        case SDLK_1: keyChar = '!'; break;
                        case SDLK_2: keyChar = '@'; break;
                        case SDLK_3: keyChar = '#'; break;
                        case SDLK_4: keyChar = '$'; break;
                        case SDLK_5: keyChar = '%'; break;
                        case SDLK_6: keyChar = '^'; break;
                        case SDLK_7: keyChar = '&'; break;
                        case SDLK_8: keyChar = '*'; break;
                        case SDLK_9: keyChar = '('; break;
                        case SDLK_0: keyChar = ')'; break;
                        case SDLK_MINUS: keyChar = '_'; break;
                        case SDLK_EQUALS: keyChar = '+'; break;
                        case SDLK_LEFTBRACKET: keyChar = '{'; break;
                        case SDLK_RIGHTBRACKET: keyChar = '}'; break;
                        case SDLK_BACKSLASH: keyChar = '|'; break;
                        case SDLK_SEMICOLON: keyChar = ':'; break;
                        case SDLK_QUOTE: keyChar = '"'; break;
                        case SDLK_COMMA: keyChar = '<'; break;
                        case SDLK_PERIOD: keyChar = '>'; break;
                        case SDLK_SLASH: keyChar = '?'; break;
                        default:
                            if (key >= SDLK_a && key <= SDLK_z) {
                                keyChar = (char)(key - 32); // Convert to uppercase
                            }
                            break;
                    }
                }
                strncat(room_name, &keyChar, 1);
            } else if (key == SDLK_BACKSPACE && strlen(room_name) > 0) {
                room_name[strlen(room_name) - 1] = '\0';
                printf("Room name: %s\n", room_name);
            }
        }
    }
}

void renderJoinRoomScreen(SDL_Renderer *renderer, TTF_Font *font, const char *room_name) {
    SDL_Color white = {255, 255, 255};
    SDL_Color black = {0, 0, 0};

    SDL_SetRenderDrawColor(renderer, black.r, black.g, black.b, 255);
    SDL_RenderClear(renderer);

    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;

    // Render "Join Room" title
    surface = TTF_RenderText_Solid(font, "Join Room", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.x = SCREEN_WIDTH / 2 - surface->w / 2;
    rect.y = 50;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render room name label
    surface = TTF_RenderText_Solid(font, "Room Name:", white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.x = SCREEN_WIDTH / 2 - surface->w / 2;
    rect.y = 150;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render room name input
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect roomNameRect = {SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 60, 200, 30};
    SDL_RenderDrawRect(renderer, &roomNameRect);
    const char *roomNameText = strlen(room_name) > 0 ? room_name : "Enter room name";
    surface = TTF_RenderText_Solid(font, roomNameText, white);
    if (!surface) {
        printf("TTF_RenderText_Solid Error: %s\n", TTF_GetError());
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }
    rect.x = roomNameRect.x + 5;
    rect.y = roomNameRect.y + 5;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

void handleJoinRandomRoomEvents(int client_fd, char *username) {
    join_random_room(client_fd, username, session_id);
}

void renderWaitingRoom(SDL_Renderer *renderer, TTF_Font *font, const char *room_name, int time_limit, int brick_limit, int max_players, const char *room_players) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect rect;
    char buffer[256];

    // Render room name
    snprintf(buffer, sizeof(buffer), "Room Name: %s", room_name);
    surface = TTF_RenderText_Solid(font, buffer, white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.x = 50;
    rect.y = 50;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render time limit
    snprintf(buffer, sizeof(buffer), "Time Limit: %d", time_limit);
    surface = TTF_RenderText_Solid(font, buffer, white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.y += 50;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render brick limit
    snprintf(buffer, sizeof(buffer), "Brick Limit: %d", brick_limit);
    surface = TTF_RenderText_Solid(font, buffer, white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.y += 50;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render max players
    snprintf(buffer, sizeof(buffer), "Max Players: %d", max_players);
    surface = TTF_RenderText_Solid(font, buffer, white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.y += 50;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Render room players
    snprintf(buffer, sizeof(buffer), "Room Players: %s", room_players);
    surface = TTF_RenderText_Solid(font, buffer, white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.y += 50;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    // Start room message
    snprintf(buffer, sizeof(buffer), "Press Enter to start the game!");
    surface = TTF_RenderText_Solid(font, buffer, white);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    rect.y += 50;
    rect.w = surface->w;
    rect.h = surface->h;
    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_DestroyTexture(texture);

    SDL_RenderPresent(renderer);
}

void handleWaitingRoomEvents(int *quit, int client_fd, const char *username) {
    SDL_Event e;
    write_to_log("Enter handle Wait Room...");
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = 1;
        } else if (e.type == SDL_KEYDOWN) {
            SDL_Keycode key = e.key.keysym.sym;
            switch (key)
            {
            case SDLK_RETURN:
                start_game(client_fd, username);
                break;
            default:
                write_to_log_int(key);
                break;
            }
        }
    }
}



