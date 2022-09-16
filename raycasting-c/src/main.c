#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>

#include "constants.h"

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int isGameRunning = FALSE;

int ticksLastFrame = 0;
int playerX, playerY;

int initializeWindow() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Error initializing SDL.\n");
    return FALSE;
  }
  window = SDL_CreateWindow("Raycasting", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_BORDERLESS);
  if (!window) {
    fprintf(stderr, "Error criating window.\n");
    return FALSE;
  }

  renderer = SDL_CreateRenderer(window, -1, 0);
  if (!renderer) {
    fprintf(stderr, "Error criating renderer.\n");
    return FALSE;
  }
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  return TRUE;
}

void destroyWindow() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void setup() {
  playerX = 0;
  playerY = 0;
}

void processInput() {
  SDL_Event event;
  SDL_PollEvent(&event);
  switch (event.type) {
  case SDL_QUIT:
    isGameRunning = FALSE;
    break;
  case SDL_KEYDOWN:
    if (event.key.keysym.sym == SDLK_ESCAPE) {
      isGameRunning = FALSE;
    }
    break;

  default:
    break;
  }
}

void update() {
  float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

  playerX += 100 * deltaTime;
  playerY += 100 * deltaTime;

  ticksLastFrame = SDL_GetTicks();
}

void render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
  SDL_Rect rect = {playerX, playerY, 20, 20};
  SDL_RenderFillRect(renderer, &rect);

  SDL_RenderPresent(renderer);
}

int main(int argc, char **argv) {
  isGameRunning = initializeWindow();

  setup();

  while (isGameRunning) {
    processInput();
    update();
    render();
  }

  destroyWindow();

  return 0;
}