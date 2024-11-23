#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../protocol/network.c"
#include "../protocol/protocol.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef enum { LOGIN_SCREEN, LOBBY_SCREEN } Screen;

Screen current_screen = LOGIN_SCREEN;
int server_fd;

// Function to send a message to the server
void send_message(const Message *msg) {
    send(server_fd, msg, sizeof(Message), 0);
}

// Function to receive a message from the server
Message receive_message() {
    Message msg;
    recv(server_fd, &msg, sizeof(Message), 0);
    return msg;
}

// Draw text using SDL_ttf
void draw_text(SDL_Renderer *renderer, TTF_Font *font, int x, int y, const char *text, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        printf("Text render error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Texture creation error: %s\n", SDL_GetError());
        SDL_FreeSurface(surface);
        return;
    }

    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Login screen handling
void login_screen(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Event event;
    int running = 1;
    char username[MAX_USERNAME] = "";
    char password[BUFFER_SIZE] = "";
    int username_entered = 0;

    SDL_Color text_color = {255, 255, 255, 255};

    while (running) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_text(renderer, font, 200, 200, "Enter Username:", text_color);
        draw_text(renderer, font, 200, 300, username, text_color);
        draw_text(renderer, font, 200, 400, "Enter Password:", text_color);
        draw_text(renderer, font, 200, 500, password, text_color);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    // Send login message to the server
                    Message msg = {0};
                    msg.type = LOGIN;
                    strncpy(msg.username, username, MAX_USERNAME - 1);
                    strncpy(msg.data, password, BUFFER_SIZE - 1);

                    send_message(&msg);
                    Message response = receive_message();

                    if (response.type == LOGIN_SUCCESS) {
                        printf("Server: %s\n", response.data);
                        current_screen = LOBBY_SCREEN;
                        running = 0;
                    } else {
                        printf("Login failed: %s\n", response.data);
                    }
                } else if (event.key.keysym.sym == SDLK_TAB) {
                    username_entered = !username_entered; // Toggle input field
                } else if (event.key.keysym.sym == SDLK_BACKSPACE) {
                    if (username_entered && strlen(username) > 0) {
                        username[strlen(username) - 1] = '\0';
                    } else if (!username_entered && strlen(password) > 0) {
                        password[strlen(password) - 1] = '\0';
                    }
                } else {
                    char key = (char)event.key.keysym.sym;
                    if (username_entered && strlen(username) < MAX_USERNAME - 1) {
                        strncat(username, &key, 1);
                    } else if (!username_entered && strlen(password) < BUFFER_SIZE - 1) {
                        strncat(password, &key, 1);
                    }
                }
            }
        }
    }
}

// Lobby screen handling
void lobby_screen(SDL_Renderer *renderer, TTF_Font *font) {
    SDL_Event event;
    int running = 1;

    SDL_Color text_color = {255, 255, 255, 255};

    while (running) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_text(renderer, font, 200, 200, "Lobby", text_color);
        draw_text(renderer, font, 200, 300, "Press C to create a room", text_color);
        draw_text(renderer, font, 200, 400, "Press J to join a room", text_color);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_c) {
                    Message msg = {0};
                    msg.type = CREATE_ROOM;
                    strncpy(msg.username, "player1", MAX_USERNAME - 1); // Replace with logged-in username
                    strncpy(msg.room_name, "Room1", MAX_ROOM_NAME - 1); // Example room name

                    send_message(&msg);
                    Message response = receive_message();
                    if (response.type == ROOM_LIST) {
                        printf("Room created successfully: %s\n", response.data);
                    } else {
                        printf("Failed to create room: %s\n", response.data);
                    }
                } else if (event.key.keysym.sym == SDLK_j) {
                    Message msg = {0};
                    msg.type = JOIN_ROOM;
                    strncpy(msg.username, "player1", MAX_USERNAME - 1); // Replace with logged-in username
                    strncpy(msg.room_name, "Room1", MAX_ROOM_NAME - 1); // Room to join

                    send_message(&msg);
                    Message response = receive_message();
                    if (response.type == ROOM_JOINED) {
                        printf("Joined room successfully: %s\n", response.data);
                    } else if (response.type == ROOM_NOT_FOUND) {
                        printf("Room not found: %s\n", response.data);
                    } else {
                        printf("Failed to join room: %s\n", response.data);
                    }
                }
            }
        }
    }
}



int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize TTF
    if (TTF_Init() == -1) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Tetris Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    // Load font
    TTF_Font *font = TTF_OpenFont("../../assets/fonts/Roboto-Black.ttf", 24);
    if (!font) {
        printf("Error loading font: %s\n", TTF_GetError());
        return 1;
    }

    // Prepare text to render
    const char *text = "Welcome to Tetris!";
    if (strlen(text) == 0) {
        printf("Text is empty or null, cannot render.\n");
        return 1;
    }

    // Render text
    SDL_Color color = {255, 255, 255};  // White text
    SDL_Surface *text_surface = TTF_RenderText_Solid(font, text, color);
    if (!text_surface) {
        printf("Text render error: %s\n", TTF_GetError());
        //return 1;
    }

    // Create texture from surface
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);  // Free the surface after creating texture

    if (!text_texture) {
        printf("Error creating texture: %s\n", SDL_GetError());
        //return 1;
    }

    // Clear the screen
    SDL_RenderClear(renderer);

    // Render the texture
    SDL_RenderCopy(renderer, text_texture, NULL, NULL);

    // Present the renderer
    SDL_RenderPresent(renderer);

    // Wait for a while
    SDL_Delay(3000);

    // Clean up
    SDL_DestroyTexture(text_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}