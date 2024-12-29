#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include "tetris_game.h"
#include "protocol/network.h"
#include "protocol/protocol.h"
#include "ultis.h"
#include <pthread.h>
#include <unistd.h> 

#define SCREEN_WIDTH  800  // or whatever value you need
#define SCREEN_HEIGHT 600  // or whatever value you need

int quit = 0;
int client_fd;

char username[MAX_USERNAME] = "";
char password[MAX_PASSWORD] = "";
char confirmPassword[MAX_PASSWORD] = "";
char room_name[255] = "";
int time_limit = 0;
int brick_limit = 0;
int max_player = 8;
int usernameSelected = 1;
int loginSuccess = 0;
int showRegisterScreen = 0;
int registerSuccess = 0;
int createRoomSuccess = 0;
int joinRoomSuccess = 0;
int selectedField = 0;
int startGame = 0;


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



// On clicking Ctr + C in terminal
void handle_signal(int sig) {
    printf("\nReceived signal %d, shutting down...\n", sig);
    handleDisconnect(client_fd, username);
    if (client_fd > 0) {
        close(client_fd);
    }
    exit(0);
}


void get_shape_list_int(int* shapeList, const Message* message) {
    if (message == NULL || shapeList == NULL) {
        write_to_log("get_shape_list_int: Invalid input parameters");
        return;
    }

    // Find the "Shapes:" section in the message data
    const char *shapes_section = strstr(message->data, "Shapes:\n");
    if (shapes_section == NULL) {
        write_to_log("get_shape_list_int: Shapes section not found in message data");
        return;
    }

    // Move the pointer to the start of the shape list length
    shapes_section += strlen("Shapes:\n");

    // Extract the count of shapes
    int count;
    sscanf(shapes_section, "%d\n", &count);

    // Move the pointer to the start of the shape list data
    const char *shape_data = strchr(shapes_section, '\n') + 1;

    // Extract the shape list integers
    for (int i = 0; i < count; i++) {
        sscanf(shape_data, "%d", &shapeList[i]);
        shape_data = strchr(shape_data, ',');
        if (shape_data != NULL) {
            shape_data++;
        }
    }

    // Add -1 at the end of the shape list
    shapeList[count] = -1;

    write_to_log("get_shape_list_int: Shape list extracted successfully");
}


void convertIntToShapeList(int* shapeListInt) {
    shapeList.count = 0;
    shapeList.current = 0;

    int i = 0;
    while (shapeListInt[i] != -1) {
        Shape shape;
        shape.width = 4;
        shape.row = 4;
        shape.col = 4;
        shape.array = malloc(4 * sizeof(int*));
        for (int j = 0; j < 4; j++) {
            shape.array[j] = malloc(4 * sizeof(int));
            for (int k = 0; k < 4; k++) {
                shape.array[j][k] = ShapesArray[shapeListInt[i]][j][k];
            }
        }
        shapeList.shapes[shapeList.count++] = shape;
        i++;
    }
}


int shapeListInt[MAX_SHAPES * 16];












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

            case START_GAME_SUCCESS:
                printf("Server: %s\n", response.data);
                get_shape_list_int(shapeListInt, &response);

                startGame = 1;
                write_to_log("Received START_GAME_SUCCESS response from server.");
                break;
            case START_GAME_FAILURE:
                printf("Server: %s\n", response.data);
                ("Failed to start the game: %s\n", response.data);
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

void startTetrisGame(SDL_Renderer *renderer, TTF_Font *font, SDL_Window *window, int time_limit, int brick_limit, const char *roomPlayers, int client_fd) {
    //initShapeList();
    //generateShapes(brick_limit);
    convertIntToShapeList(shapeListInt);
    newRandomShape2();

    // Main game loop
    int lastTime = SDL_GetTicks();
    int startTime = lastTime;
    int brickPlaced = 0;
    int shapeMovedDown = 0;

    printf("Starting Tetris game with time limit: %d seconds and brick limit: %d bricks\n", time_limit, brick_limit);

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

        printf("Current GameOn = %d", GameOn);
        shapeMovedDown = 0;
        if (currentTime - lastTime > timer) {
            if (moveShapeDown(client_fd, username)) {
                brickPlaced++;
                printf("Brick placed: %d, Current time: %d, Last time: %d\n", brickPlaced, currentTime, lastTime);
            }
            lastTime = currentTime;
            shapeMovedDown = 1;
        }
        handleEvents(&quit, client_fd, username, &shapeMovedDown);
        renderGame(renderer, font, roomPlayers);
    }

    freeShape(current);
    printf("Shape freed.\n");


    // Print final score
    printf("Game Over! Final Score: %d\n", score);
}


int main() {
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    client_fd = create_client_socket(server_ip);
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

    // Auth section
    while (!quit && !loginSuccess) {
        if (showRegisterScreen) {
            handleRegisterEvents(&quit, &registerSuccess, username, password, confirmPassword, &selectedField, client_fd);
            renderRegisterScreen(renderer, font, username, password, confirmPassword, selectedField);
            if (registerSuccess) {
                showRegisterScreen = 0;
                registerSuccess = 0;
                memset(username, 0, sizeof(username));
                memset(password, 0, sizeof(password));
                memset(confirmPassword, 0, sizeof(confirmPassword));
            }
        } else {
            handleLoginEvents(&quit, &loginSuccess, &showRegisterScreen, username, password, &usernameSelected, client_fd);
            renderLoginScreen(renderer, font, username, password, usernameSelected);
        }
    }

    // In game section

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
                handleWaitingRoomEvents(&quit, client_fd, username);
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
                startTetrisGame(renderer, font, window, currentTimeLimit, currentBrickLimit, currentRoomPlayers, client_fd);
            }
        }
        // Cleanup
        if (renderer) {
            SDL_DestroyRenderer(renderer);
            printf("Renderer destroyed.\n");
        }
        if (window) {
            SDL_DestroyWindow(window);
            printf("Window destroyed.\n");

        }
        if (font) {
            TTF_CloseFont(font);
            printf("Font closed.\n");

        }
        handleDisconnect(client_fd, username);
        TTF_Quit();
        SDL_Quit();
    }
    return 0;
}