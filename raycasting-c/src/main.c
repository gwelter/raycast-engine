#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
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

struct Ray {
  float rayAngle;
  float wallHitX;
  float wallHitY;
  double distance;
  int wasHitVertical;
  int wallHitContent;
  int isRayFacingUp;
  int isRayFacingDown;
  int isRayFacingLeft;
  int isRayFacingRight;
} rays[NUM_RAYS];

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
  player.width = 1;
  player.height = 1;
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

int hasWallAt(float x, float y) {
  if (x <= 0 || x >= SCREEN_WIDTH || y <= 0 || y >= SCREEN_HEIGHT)
    return TRUE;

  int i = floor(y / TILE_SIZE);
  int j = floor(x / TILE_SIZE);
  return map[i][j] != 0;
}

void movePlayer(float deltatime) {
  player.rotationAngle += player.turnDirection * player.turnSpeed * deltatime;

  float moveStep = player.walkSpeed * player.walkDirection * deltatime;
  float nextX = player.x + cos(player.rotationAngle) * moveStep;
  float nextY = player.y + sin(player.rotationAngle) * moveStep;

  if (hasWallAt(nextX, nextY))
    return;

  player.x = nextX;
  player.y = nextY;
}

double distanceBetweenPoints(float x1, float y1, float x2, float y2) {
  float x = (x2 - x1) * (x2 - x1);
  float y = (y2 - y1) * (y2 - y1);
  return sqrt(x + y);
}

float normalizeAngle(float angle) {
  angle = remainder(angle, TWO_PI);
  if (angle < 0) {
    angle = TWO_PI + angle;
  }
  return angle;
}

void horizontalInterception(float rayAngle, int isRayFacingDown,
                            float *xintercept, float *yintercept) {
  *yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
  *yintercept = *yintercept + isRayFacingDown ? TILE_SIZE : 0;

  int opositeSide = *yintercept - player.y;

  *xintercept = player.x + opositeSide / tan(rayAngle);
}

void horizontalStep(float rayAngle, int isRayFacingUp, int isRayFacingLeft,
                    int isRayFacingRight, float xintercept, float yintercept,
                    float *x, float *y) {

  int foundWallHit = FALSE;
  int wallContent;
  float xstep, ystep;
  float nextXTouch = xintercept;
  float nextYTouch = yintercept;

  ystep = TILE_SIZE;
  ystep *= isRayFacingUp ? -1 : 1;

  xstep = TILE_SIZE / tan(rayAngle);
  xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
  xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

  while (!foundWallHit) {
    float xToCheck = nextXTouch;
    float yToCheck = nextYTouch + (isRayFacingUp ? -1 : 0);
    if (hasWallAt(xToCheck, yToCheck)) {
      wallContent = map[(int)floor(yToCheck / TILE_SIZE)]
                       [(int)floor(xToCheck / TILE_SIZE)];
      nextXTouch = xstep;
      nextYTouch = ystep;
      foundWallHit = TRUE;
      break;
    } else {
      nextXTouch += xstep;
      nextYTouch += ystep;
    }
  }
  *x = nextXTouch;
  *y = nextYTouch;
}

void verticalInterception(float rayAngle, int isRayFacingLeft,
                          int isRayFacingRight, float *xintercept,
                          float *yintercept) {
  *xintercept = 0;
  *yintercept = 0;
  *xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
  *xintercept += isRayFacingRight ? TILE_SIZE : 0;

  float adjecentSide = *xintercept - player.x;

  *yintercept = player.y + adjecentSide * tan(rayAngle);
}

void verticalStep(float rayAngle, int isRayFacingLeft, int isRayFacingUp,
                  int isRayFacingDown, float xintercept, float yintercept,
                  float *x, float *y) {
  int foundWallHit = FALSE;
  int wallContent;
  float xstep, ystep = 0;
  float nextXTouch = xintercept;
  float nextYTouch = yintercept;

  xstep = TILE_SIZE;
  xstep *= isRayFacingLeft ? -1 : 1;

  ystep = TILE_SIZE * tan(rayAngle);
  ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
  ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

  while (!foundWallHit) {
    float xToCheck = nextXTouch - (isRayFacingLeft ? 1 : 0);
    float yToCheck = nextYTouch;
    if (hasWallAt(nextXTouch, nextYTouch)) {
      foundWallHit = TRUE;
      wallContent = map[(int)floor(yToCheck / TILE_SIZE)]
                       [(int)floor(xToCheck / TILE_SIZE)];
      nextXTouch = xstep;
      nextYTouch = ystep;
      foundWallHit = TRUE;
      break;
    } else {
      nextXTouch += xstep;
      nextYTouch += ystep;
    }
  }
  *x = nextXTouch;
  *y = nextYTouch;
}

void castRay(float rayAngle, int stripId) {
  rayAngle = normalizeAngle(rayAngle);
  int isRayFacingDown = rayAngle > 0 && rayAngle < PI;
  int isRayFacingUp = !isRayFacingDown;
  int isRayFacingRight = rayAngle < (PI * 0.5) || rayAngle > (PI * 1.5);
  int isRayFacingLeft = !isRayFacingRight;

  int horzWallContent, vertWallContent;
  float horzXintercept, horzYintercept;
  float vertXintercept, vertYintercept;
  float horzWallHitX, horzWallHitY;
  float vertWallHitX, vertWallHitY;

  horizontalInterception(rayAngle, isRayFacingDown, &horzXintercept,
                         &horzYintercept);

  horizontalStep(rayAngle, isRayFacingUp, isRayFacingLeft, isRayFacingRight,
                 horzXintercept, horzYintercept, &horzWallHitX, &horzWallHitY);

  verticalInterception(rayAngle, isRayFacingLeft, isRayFacingRight,
                       &vertXintercept, &vertYintercept);

  verticalStep(rayAngle, isRayFacingLeft, isRayFacingUp, isRayFacingDown,
               vertXintercept, vertYintercept, &vertWallHitX, &vertWallHitY);

  double horizontalDistance =
      distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY);
  double verticalDistance =
      distanceBetweenPoints(player.x, player.y, vertWallHitY, vertWallHitY);

  int wasHitVertical = verticalDistance < horizontalDistance;
  if (wasHitVertical) {
    rays[stripId].distance = verticalDistance;
    rays[stripId].wallHitX = vertWallHitX;
    rays[stripId].wallHitY = vertWallHitY;
  } else {
    rays[stripId].distance = horizontalDistance;
    rays[stripId].wallHitX = horzWallHitX;
    rays[stripId].wallHitY = horzWallHitY;
  }

  rays[stripId].isRayFacingUp = isRayFacingUp;
  rays[stripId].isRayFacingDown = isRayFacingDown;
  rays[stripId].isRayFacingLeft = isRayFacingLeft;
  rays[stripId].isRayFacingRight = isRayFacingRight;
  rays[stripId].rayAngle = rayAngle;
  rays[stripId].wasHitVertical = wasHitVertical;
}

void castAllRays() {
  // start first ray subtracting half of the FOV
  float rayAngle = player.rotationAngle - (FOV_ANGLE / 2);
  for (int stripId = 0; stripId < NUM_RAYS; stripId++) {
    castRay(rayAngle, stripId);

    rayAngle += FOV_ANGLE / NUM_RAYS;
  }
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

void renderRays() {
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  for (int i = 0; i < NUM_RAYS; i++) {
    SDL_RenderDrawLine(renderer, player.x * MINIMAP_SCALE_FACTOR,
                       player.y * MINIMAP_SCALE_FACTOR,
                       rays[i].wallHitX * MINIMAP_SCALE_FACTOR,
                       rays[i].wallHitY * MINIMAP_SCALE_FACTOR);
  }
}

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

void update() {
  int timeToWait = FRAME_TIME_LENGTH - (SDL_GetTicks() - ticksLastFrame);

  if (timeToWait > 0 && timeToWait <= FRAME_TIME_LENGTH) {
    SDL_Delay(timeToWait);
  }

  float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;

  movePlayer(deltaTime);
  castAllRays();

  ticksLastFrame = SDL_GetTicks();
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