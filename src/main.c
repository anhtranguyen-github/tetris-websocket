#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "tetris_game.h"

#define SCREEN_WIDTH  800  // or whatever value you need
#define SCREEN_HEIGHT 600  // or whatever value you need


int main() {

    
    initShapeList();
    generateShapes(100);
    srand(time(NULL));
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window *window = SDL_CreateWindow("SDL Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("../fonts/Roboto-Black.ttf", 24);


    
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
