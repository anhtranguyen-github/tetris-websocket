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

void renderMenu(SDL_Renderer *renderer, TTF_Font *font, Button buttons[], int buttonCount) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < buttonCount; i++) {
        renderButton(renderer, font, buttons[i]);
    }

    SDL_RenderPresent(renderer);
}

const char *server_ip = "127.0.0.1";

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

    char username[MAX_USERNAME] = "";
    char password[MAX_PASSWORD] = "";
    char room_name[255] = "";
    int time_limit = 0;
    int brick_limit = 0;
    int max_player = 8;
    int usernameSelected = 1;
    int loginSuccess = 0;
    int quit = 0;
    int createRoomSuccess = 0;
    int joinRoomSuccess = 0;
    int selectedField = 0;


    while (!quit && !loginSuccess) {
        handleLoginEvents(&quit, &loginSuccess, username, password, &usernameSelected, client_fd);
        renderLoginScreen(renderer, font, username, password, usernameSelected);
    }

    if (loginSuccess) {
        Button buttons[3] = {
            {{SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 100, 200, 50}, {0, 0, 255, 255}, "Create Room"},
            {{SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 200, 50}, {0, 255, 0, 255}, "Join Room"},
            {{SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 100, 200, 50}, {255, 0, 0, 255}, "Quick Join"}
        };
        
        int inGame = 0;

        initShapeList();
        generateShapes(100);
        newRandomShape2();

        while (!quit) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    quit = 1;
                } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    int x, y;
                    SDL_GetMouseState(&x, &y);
                    for (int i = 0; i < 3; i++) {
                        if (handleButtonClick(buttons[i], x, y)) {
                            printf("%s button clicked\n", buttons[i].text);
                            // Handle button actions here
                            if (strcmp(buttons[i].text, "Create Room") == 0) {
                                // Handle create room
                                while (!quit && !createRoomSuccess) {
                                    handleCreateRoomEvents(&quit, &createRoomSuccess, username, room_name, &time_limit, &brick_limit, &max_player, &selectedField, client_fd);
                                    renderCreateRoomScreen(renderer, font, username, room_name, time_limit, brick_limit, max_player, selectedField);
                                }
                            } else if (strcmp(buttons[i].text, "Join Room") == 0) {
                                // Handle join room
                            } else if (strcmp(buttons[i].text, "Quick Join") == 0) {
                                // Handle quick join
                                Message response;
                                while (!quit && !joinRoomSuccess) {
                                    handleJoinRandomRoomEvents(&quit, &joinRoomSuccess, username, client_fd, &response);
                                }
                                if (joinRoomSuccess) {
                                    char room_name[MAX_ROOM_NAME];
                                    int time_limit, brick_limit, max_players;
                                    char room_players[ROOM_PLAYER_BUFFER_SIZE];
                            
                                    // Parse the response data to extract room details
                                    sscanf(response.data, "%[^|]|%d|%d|%d|%[^\n]", room_name, &time_limit, &brick_limit, &max_players, room_players);
                            
                                    // Render the waiting room screen
                                    renderWaitingRoom(renderer, font, room_name, time_limit, brick_limit, max_players, room_players);
                                }
                            }
                        }
                    }
                }
            }

            if (inGame) {
                int quit = 0;
                int lastTime = SDL_GetTicks();
                while (GameOn && !quit) {
                    int currentTime = SDL_GetTicks();
                    if (currentTime - lastTime > timer) {
                        moveShapeDown();
                        lastTime = currentTime;
                    }
                    handleEvents(&quit);
                    renderGame(renderer, font);
                }
                
            } else {
                renderMenu(renderer, font, buttons, 3);
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
