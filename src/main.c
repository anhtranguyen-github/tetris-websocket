#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "tetris_game.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080


#define SCREEN_WIDTH  800  // or whatever value you need
#define SCREEN_HEIGHT 600  // or whatever value you need


int sock;
char username[32];




void connectToServer() {
    struct sockaddr_in server;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        //exit(1);
    }

    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Connection failed\n");
        //exit(1);
    }
    printf("Connected to server\n");
}

void sendToServer(const char* message) {
    if (send(sock, message, strlen(message), 0) < 0) {
        printf("Send failed\n");
        //exit(1);
    }
}

void receiveFromServer(char* buffer, size_t size) {
    if (recv(sock, buffer, size, 0) < 0) {
        printf("Receive failed\n");
        //exit(1);
    }
}




void loginScreen(SDL_Renderer* renderer, TTF_Font* font) {
    int loggedIn = 0;
    SDL_StartTextInput();
    char input[32] = {0};
    while (!loggedIn) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            //if (event.type == SDL_QUIT) exit(0);
            if (event.type == SDL_TEXTINPUT) {
                strcat(input, event.text.text);
            } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_RETURN) {
                strcpy(username, input);
                char message[64];
                sprintf(message, "LOGIN:%s", username);
                sendToServer(message);
                loggedIn = 1;
            }
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render text
        SDL_Color white = {255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Solid(font, "Enter Username:", white);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect dest = {200, 200, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        surface = TTF_RenderText_Solid(font, input, white);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        dest.y += 50;
        dest.w = surface->w;
        dest.h = surface->h;
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        SDL_RenderPresent(renderer);
    }
    SDL_StopTextInput();
}
void lobby(SDL_Renderer* renderer, TTF_Font* font) {
    char buffer[512];
    int inLobby = 1;

    while (inLobby) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            //if (event.type == SDL_QUIT) exit(0);
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_1) {
                    sendToServer("CREATE_ROOM");
                } else if (event.key.keysym.sym == SDLK_2) {
                    sendToServer("JOIN_ROOM");
                }
            }
        }

        // Receive server messages
        receiveFromServer(buffer, sizeof(buffer));
        if (strcmp(buffer, "ROOM_CREATED") == 0) {
            printf("Room created\n");
            inLobby = 0;
        } else if (strcmp(buffer, "ROOM_JOINED") == 0) {
            printf("Joined room\n");
            inLobby = 0;
        }

        // Render lobby screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Color white = {255, 255, 255};
        SDL_Surface* surface = TTF_RenderText_Solid(font, "1. Create Room", white);
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect dest = {200, 200, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        surface = TTF_RenderText_Solid(font, "2. Join Room", white);
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        dest.y += 50;
        dest.w = surface->w;
        dest.h = surface->h;
        SDL_RenderCopy(renderer, texture, NULL, &dest);
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);

        SDL_RenderPresent(renderer);
    }
}



int main() {

    
    
    
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window *window = SDL_CreateWindow("SDL Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("../fonts/Roboto-Black.ttf", 24);


    connectToServer();
    loginScreen(renderer, font);
    lobby(renderer, font);


    initShapeList();
    generateShapes(100);
    newRandomShape2();
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
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    freeShape(current);
    printf("Game Over! Final Score: %d\n", score);
    return 0;
}
