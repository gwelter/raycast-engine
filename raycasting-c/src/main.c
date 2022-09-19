#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <stdio.h>

#include "constants.h"

const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

struct Player {
  float x;
  float y;
  float width;
  float height;
  float rotationAngle;
  float walkSpeed;
  float turnSpeed;
  int turnDirection; // -1 left - 1 right
  int walkDirection; // -1 for back - 1 for front
} player;

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
  player.x = SCREEN_WIDTH / 2;
  player.y = SCREEN_HEIGHT / 2;
  player.width = 5;
  player.height = 5;
  player.turnDirection = 0;
  player.walkDirection = 0;
  player.rotationAngle = PI / 2.0f;
  player.walkSpeed = 150;
  player.turnSpeed = 100 * PI / 180;
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
    if (event.key.keysym.sym == SDLK_UP) {
      player.walkDirection = 1;
    }
    if (event.key.keysym.sym == SDLK_DOWN) {
      player.walkDirection = -1;
    }
    if (event.key.keysym.sym == SDLK_RIGHT) {
      player.turnDirection = 1;
    }
    if (event.key.keysym.sym == SDLK_LEFT) {
      player.turnDirection = -1;
    }
    break;
  case SDL_KEYUP:
    if (event.key.keysym.sym == SDLK_UP) {
      player.walkDirection = 0;
    }
    if (event.key.keysym.sym == SDLK_DOWN) {
      player.walkDirection = 0;
    }
    if (event.key.keysym.sym == SDLK_RIGHT) {
      player.turnDirection = 0;
    }
    if (event.key.keysym.sym == SDLK_LEFT) {
      player.turnDirection = 0;
    }
    break;

  default:
    break;
  }
}

void movePlayer(float deltatime) {
  player.rotationAngle += player.turnDirection * player.turnSpeed * deltatime;

  float moveStep = player.walkSpeed * player.walkDirection * deltatime;
  player.x = player.x + cos(player.rotationAngle) * moveStep;
  player.y = player.y + sin(player.rotationAngle) * moveStep;
}

void update() {
  float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

  movePlayer(deltaTime);

  ticksLastFrame = SDL_GetTicks();
}

void renderMap() {
  for (int i = 0; i < MAP_NUM_ROWS; i++) {
    for (int j = 0; j < MAP_NUM_COLS; j++) {
      int tileX = j * TILE_SIZE;
      int tileY = i * TILE_SIZE;
      int tileColor = map[i][j] != 0 ? 255 : 0;

      SDL_SetRenderDrawColor(renderer, tileColor, tileColor, tileColor, 255);
      SDL_FRect mapTileRect = {
          tileX * MINIMAP_SCALE_FACTOR, tileY * MINIMAP_SCALE_FACTOR,
          TILE_SIZE * MINIMAP_SCALE_FACTOR, TILE_SIZE * MINIMAP_SCALE_FACTOR};

      SDL_RenderFillRectF(renderer, &mapTileRect);
    }
  }
}
void renderRays() {}

void renderPlayer() {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_Rect playerRect = {
      player.x * MINIMAP_SCALE_FACTOR,
      player.y * MINIMAP_SCALE_FACTOR,
      player.width * MINIMAP_SCALE_FACTOR,
      player.height * MINIMAP_SCALE_FACTOR,
  };
  SDL_RenderFillRect(renderer, &playerRect);
  SDL_RenderDrawLine(
      renderer, player.x * MINIMAP_SCALE_FACTOR,
      player.y * MINIMAP_SCALE_FACTOR,
      player.x + cos(player.rotationAngle) * 40 * MINIMAP_SCALE_FACTOR,
      player.y + sin(player.rotationAngle) * 40 * MINIMAP_SCALE_FACTOR);
}

void render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  renderMap();
  renderRays();
  renderPlayer();

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