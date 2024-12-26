#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "tetris_game.h"
#include "protocol/network.h"
#include "protocol/protocol.h"
#include "ultis.h"

#define SCREEN_WIDTH  800  // or whatever value you need
#define SCREEN_HEIGHT 600  // or whatever value you need

int quit = 0;
int client_fd;

char username[MAX_USERNAME] = "";
char password[MAX_PASSWORD] = "";
char room_name[255] = "";
int time_limit = 0;
int brick_limit = 0;
int max_player = 8;
int usernameSelected = 1;
int loginSuccess = 0;
int createRoomSuccess = 0;
int joinRoomSuccess = 0;
int selectedField = 0;


void renderMenu(SDL_Renderer *renderer, TTF_Font *font, Button buttons[], int buttonCount) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < buttonCount; i++) {
        renderButton(renderer, font, buttons[i]);
    }

    SDL_RenderPresent(renderer);
}

const char *server_ip = "127.0.0.1";

typedef enum {
    MENU_SCREEN,
    WAITING_ROOM_SCREEN,
    GAME_SCREEN
} ScreenState;

char currentRoomName[MAX_ROOM_NAME];
int currentTimeLimit;
int currentBrickLimit;
int currentMaxPlayers;
char currentRoomPlayers[BUFFER_SIZE];
ScreenState currentScreen;

void handleServerMessages(int client_fd) {
    Message response;
    while (1) {
        int bytes_received = recv(client_fd, &response, sizeof(Message), 0);
        if (bytes_received <= 0) {
            printf("Error receiving message from server.\n");
            break;
        }

        switch (response.type) {
            case CREATE_ROOM_SUCCESS:
                printf("Room created successfully: %s\n", response.data);
                createRoomSuccess = 1;
                break;
            case CREATE_ROOM_FAILURE:
                printf("Failed to create room: %s\n", response.data);
                break;
            case ROOM_JOINED:
                printf("Successfully joined room: %s\n", response.room_name);
                printf("Server message: %s\n", response.data);
                // Update screen state and render waiting room
                sscanf(response.data, "%[^|]|%d|%d|%d|%[^|]",
                       currentRoomName, &currentTimeLimit, &currentBrickLimit, 
                       &currentMaxPlayers, currentRoomPlayers);
                joinRoomSuccess = 1;
                currentScreen = WAITING_ROOM_SCREEN;
                break;

            case JOIN_ROOM_FAILURE:
                printf("Failed to join room: %s\n", response.data);
                break;

            case ROOM_NOT_FOUND:
                printf("Room not found: %s\n", response.data);
                break;

            case ROOM_FULL:
                printf("Room is full: %s\n", response.data);
                break;
            case PLAYER_JOINED:
                printf("Another player joined: %s\n", response.data);
                break;               

            case GAME_ALREADY_STARTED:
                printf("Game already started in room: %s\n", response.data);
                break;

            default:
                printf("Unexpected response type: %d\n", response.type);
                break;
        }
    }
}

void* serverMessageThread(void* arg) {
    int client_fd = *((int*)arg);
    handleServerMessages(client_fd);
    return NULL;
}

void startTetrisGame(SDL_Renderer *renderer, TTF_Font *font, SDL_Window *window, int time_limit, int brick_limit, const char *roomPlayers) {
    initShapeList();
    generateShapes(brick_limit);
    newRandomShape2();

    // Main game loop
    int lastTime = SDL_GetTicks();
    int startTime = lastTime;
    int brickPlaced = 0;

    while (GameOn && !quit) {
        int currentTime = SDL_GetTicks();
        //Check time limit
        if ((currentTime - startTime) / 1000 >= time_limit) {
            printf("Time limit reached!\n");
            break;
        }

        // Check block limit
        if (brickPlaced >= brick_limit) {
            printf("Brick limit reached!\n");
            break;
        }

        if (currentTime - lastTime > timer) {
            moveShapeDown();
            brickPlaced++;
            lastTime = currentTime;
        }
        handleEvents(&quit);
        renderGame(renderer, font, roomPlayers);
    }

    // Cleanup
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        printf("Rendere destroyed.\n");
    }
    if (window) {
        SDL_DestroyWindow(window);
        printf("Window destroyed.\n");

    }
    if (font) {
        TTF_CloseFont(font);
        printf("Font closed.\n");

    }
    TTF_Quit();
    SDL_Quit();
    freeShape(current);
    printf("Shape freed.\n");


    // Print final score
    printf("Game Over! Final Score: %d\n", score);
}


int main() {
    int client_fd = create_client_socket(server_ip);
    if (client_fd < 0) {
        fprintf(stderr, "Client: Failed to connect to server\n");
        exit(EXIT_FAILURE);
    }
    printf("Starting Tetris...\n");
    srand(time(NULL));
    
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL initialized.\n");

    if (TTF_Init() == -1) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    printf("SDL_ttf initialized.\n");

    SDL_Window *window = SDL_CreateWindow("SDL Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    printf("Window created.\n");

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    printf("Renderer created.\n");

    TTF_Font *font = TTF_OpenFont("../assets/fonts/Roboto-Black.ttf", 24);
    if (font == NULL) {
        printf("TTF_OpenFont Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    printf("Font loaded.\n");

    while (!quit && !loginSuccess) {
        handleLoginEvents(&quit, &loginSuccess, username, password, &usernameSelected, client_fd);
        renderLoginScreen(renderer, font, username, password, usernameSelected);
    }

    pthread_t tid;
    if (pthread_create(&tid, NULL, serverMessageThread, &client_fd) != 0) {
        perror("Failed to create server message thread");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if (loginSuccess) {
        currentScreen = MENU_SCREEN;
        Button buttons[3] = {
            {{SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, 200, 50}, {0, 0, 255, 255}, "Create Room"},
            {{SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 200, 50}, {0, 255, 0, 255}, "Join Room"},
            {{SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 100, 200, 50}, {255, 0, 0, 255}, "Quick Join"}
        };
        

        while (!quit) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = 1;
                } else if (e.type == SDL_MOUSEBUTTONDOWN && currentScreen == MENU_SCREEN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    for (int i = 0; i < 3; i++) {
                        if (handleButtonClick(buttons[i], x, y)) {
                            printf("%s button clicked\n", buttons[i].text);
                            // Handle button actions here
                            if (strcmp(buttons[i].text, "Create Room") == 0) {
                                // Handle create room
                                while (!quit && !createRoomSuccess) {
                                    handleCreateRoomEvents(&quit, username, room_name, &time_limit, &brick_limit, &max_player, &selectedField, client_fd);
                                    renderCreateRoomScreen(renderer, font, username, room_name, time_limit, brick_limit, max_player, selectedField);
                                }

                                if (createRoomSuccess) {
                                    // Initialize waiting room data
                                    strncpy(currentRoomName, room_name, MAX_ROOM_NAME);
                                    currentTimeLimit = time_limit;
                                    currentBrickLimit = brick_limit;
                                    currentMaxPlayers = max_player;
                                    snprintf(currentRoomPlayers, sizeof(currentRoomPlayers), "%s", username); // Add creator as the first player
                                    createRoomSuccess = 0;
                                    // Transition to the waiting room screen
                                    currentScreen = WAITING_ROOM_SCREEN;
                                }

                            } else if (strcmp(buttons[i].text, "Join Room") == 0) {
                                // Handle join room
                                while (!quit && !joinRoomSuccess){
                                    handleJoinRoomEvents(&quit, username, room_name, client_fd);
                                    renderJoinRoomScreen(renderer, font, room_name);
                                }

                            } else if (strcmp(buttons[i].text, "Quick Join") == 0) {
                                // Handle quick join
                                handleJoinRandomRoomEvents(client_fd, username);
                            }
                        }
                    }
                } 
            }
            if (currentScreen == MENU_SCREEN) {
                renderMenu(renderer, font, buttons, 3);
            } else if (currentScreen == WAITING_ROOM_SCREEN) {
                write_to_log("Into the wait room...");
                int startGame = 0;
                handleWaitingRoomEvents(&quit, &startGame);
                renderWaitingRoom(renderer, font, currentRoomName, currentTimeLimit, currentBrickLimit, currentMaxPlayers, currentRoomPlayers);
                // waiting room, then someone (or the host) press Enter
                    // Right now everyone gets the: "Press Enter to start Game" text
                    // If the server returns a message contains the host id, 
                    // compare user_id == host_id, only the host will get the text, and a handle Enter event
                // After press Enter, the, sends starts game to the server using START_GAME(?)
                // Server create game in the DB?
                // Server broadcast to all players in the room
                // Client recieved the message START_GAME_SUCCESS(?)
                // After recieving the message, int startGame = 1 then the following line starts:
                
                if (startGame) {
                    currentScreen = GAME_SCREEN;
                }
            } else if (currentScreen == GAME_SCREEN) {
                startTetrisGame(renderer, font, window, currentTimeLimit, currentBrickLimit, currentRoomPlayers);
            }
        }
    }
    return 0;
}