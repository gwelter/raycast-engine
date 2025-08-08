#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "constants.h"
#include "textures.h"

Uint32 lastTime;
int frameTime = 0;
int frameCount = 0;
int fps = 0;

SDL_Color whiteColor = {255, 255, 255, 255};
const int map[MAP_NUM_ROWS][MAP_NUM_COLS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 2, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 7, 3, 6, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5}
};

struct Player {
  float x;
  float y;
  float width;
  float height;
  float rotationAngle;
  float walkSpeed;
  float turnSpeed;
  int turnDirection;  // -1 left - 1 right
  int walkDirection;  // -1 for back - 1 for front
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

TTF_Font *font = NULL;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
int isGameRunning = FALSE;
int ticksLastFrame = 0;

uint32_t *colorBuffer = NULL;
uint32_t *textures[NUM_TEXTURES];
SDL_Texture *colorBufferTexture = NULL;
SDL_Surface *textSurface = NULL;
SDL_Texture *textTexture = NULL;

int initializeWindow() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Error initializing SDL.\n");
    return FALSE;
  }
  window = SDL_CreateWindow("Raycasting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,
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

  if (TTF_Init() != 0) {
    fprintf(stderr, "Error initializing TTF.\n");
    return FALSE;
  }
  font = TTF_OpenFont(MY_FONT, 24);
  if (!font) {
    fprintf(stderr, "Error opening font.\n");
    return FALSE;
  }

  return TRUE;
}

void destroyWindow() {
  free(colorBuffer);
  SDL_DestroyTexture(colorBufferTexture);
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

  colorBuffer = (uint32_t *)malloc(sizeof(uint32_t) * (uint32_t)WINDOW_WIDTH * (uint32_t)WINDOW_HEIGHT);
  colorBufferTexture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT);

  textures[0] = (uint32_t *)REDBRICK_TEXTURE;
  textures[1] = (uint32_t *)PURPLESTONE_TEXTURE;
  textures[2] = (uint32_t *)MOSSYSTONE_TEXTURE;
  textures[3] = (uint32_t *)GRAYSTONE_TEXTURE;
  textures[4] = (uint32_t *)COLORSTONE_TEXTURE;
  textures[5] = (uint32_t *)BLUESTONE_TEXTURE;
  textures[6] = (uint32_t *)WOOD_TEXTURE;
  textures[7] = (uint32_t *)EAGLE_TEXTURE;
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
  if (x <= 0 || x >= SCREEN_WIDTH || y <= 0 || y >= SCREEN_HEIGHT) return TRUE;

  int i = floor(y / TILE_SIZE);
  int j = floor(x / TILE_SIZE);
  return map[i][j] != 0;
}

int wallContentAt(float x, float y) {
  if (x <= 0 || x >= SCREEN_WIDTH || y <= 0 || y >= SCREEN_HEIGHT) return 0;

  int i = floor(y / TILE_SIZE);
  int j = floor(x / TILE_SIZE);
  return map[i][j];
}

void movePlayer(float deltatime) {
  player.rotationAngle += player.turnDirection * player.turnSpeed * deltatime;

  float moveStep = player.walkSpeed * player.walkDirection * deltatime;
  float nextX = player.x + cos(player.rotationAngle) * moveStep;
  float nextY = player.y + sin(player.rotationAngle) * moveStep;

  if (hasWallAt(nextX, nextY)) return;

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

void horizontalInterception(float rayAngle, int isRayFacingDown, float *xintercept, float *yintercept) {
  *yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
  *yintercept += isRayFacingDown ? TILE_SIZE : 0;

  int opositeSide = *yintercept - player.y;

  *xintercept = player.x + opositeSide / tan(rayAngle);
}

void horizontalStep(float rayAngle, int isRayFacingUp, int isRayFacingLeft, int isRayFacingRight, float xintercept,
                    float yintercept, float *x, float *y, int *wallHitContent) {
  int foundWallHit = FALSE;
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
    float yToCheck = nextYTouch - (isRayFacingUp ? 1 : 0);
    if (hasWallAt(xToCheck, yToCheck)) {
      *wallHitContent = wallContentAt(xToCheck, yToCheck);
      foundWallHit = TRUE;
    } else {
      nextXTouch += xstep;
      nextYTouch += ystep;
    }
  }
  *x = nextXTouch;
  *y = nextYTouch;
}

void verticalInterception(float rayAngle, int isRayFacingLeft, int isRayFacingRight, float *xintercept,
                          float *yintercept) {
  *xintercept = 0;
  *yintercept = 0;
  *xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
  *xintercept += isRayFacingRight ? TILE_SIZE : 0;

  float adjecentSide = *xintercept - player.x;

  *yintercept = player.y + adjecentSide * tan(rayAngle);
}

void verticalStep(float rayAngle, int isRayFacingLeft, int isRayFacingUp, int isRayFacingDown, float xintercept,
                  float yintercept, float *x, float *y, int *wallHitContent) {
  int foundWallHit = FALSE;
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
    if (hasWallAt(xToCheck, nextYTouch)) {
      *wallHitContent = wallContentAt(xToCheck, yToCheck);
      foundWallHit = TRUE;
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

  int isRayFacingRight = rayAngle < 0.5 * PI || rayAngle > 1.5 * PI;
  int isRayFacingLeft = !isRayFacingRight;

  float xintercept, yintercept;
  float xstep, ystep;

  ///////////////////////////////////////////
  // HORIZONTAL RAY-GRID INTERSECTION CODE
  ///////////////////////////////////////////
  int foundHorzWallHit = FALSE;
  float horzWallHitX = 0;
  float horzWallHitY = 0;
  int horzWallContent = 0;

  // Find the y-coordinate of the closest horizontal grid intersection
  yintercept = floor(player.y / TILE_SIZE) * TILE_SIZE;
  yintercept += isRayFacingDown ? TILE_SIZE : 0;

  // Find the x-coordinate of the closest horizontal grid intersection
  xintercept = player.x + (yintercept - player.y) / tan(rayAngle);

  // Calculate the increment xstep and ystep
  ystep = TILE_SIZE;
  ystep *= isRayFacingUp ? -1 : 1;

  xstep = TILE_SIZE / tan(rayAngle);
  xstep *= (isRayFacingLeft && xstep > 0) ? -1 : 1;
  xstep *= (isRayFacingRight && xstep < 0) ? -1 : 1;

  float nextHorzTouchX = xintercept;
  float nextHorzTouchY = yintercept;

  // Increment xstep and ystep until we find a wall
  while (nextHorzTouchX >= 0 && nextHorzTouchX <= WINDOW_WIDTH && nextHorzTouchY >= 0 &&
         nextHorzTouchY <= WINDOW_HEIGHT) {
    float xToCheck = nextHorzTouchX;
    float yToCheck = nextHorzTouchY + (isRayFacingUp ? -1 : 0);

    if (hasWallAt(xToCheck, yToCheck)) {
      // found a wall hit
      horzWallHitX = nextHorzTouchX;
      horzWallHitY = nextHorzTouchY;
      horzWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
      foundHorzWallHit = TRUE;
      break;
    } else {
      nextHorzTouchX += xstep;
      nextHorzTouchY += ystep;
    }
  }

  ///////////////////////////////////////////
  // VERTICAL RAY-GRID INTERSECTION CODE
  ///////////////////////////////////////////
  int foundVertWallHit = FALSE;
  float vertWallHitX = 0;
  float vertWallHitY = 0;
  int vertWallContent = 0;

  // Find the x-coordinate of the closest horizontal grid intersection
  xintercept = floor(player.x / TILE_SIZE) * TILE_SIZE;
  xintercept += isRayFacingRight ? TILE_SIZE : 0;

  // Find the y-coordinate of the closest horizontal grid intersection
  yintercept = player.y + (xintercept - player.x) * tan(rayAngle);

  // Calculate the increment xstep and ystep
  xstep = TILE_SIZE;
  xstep *= isRayFacingLeft ? -1 : 1;

  ystep = TILE_SIZE * tan(rayAngle);
  ystep *= (isRayFacingUp && ystep > 0) ? -1 : 1;
  ystep *= (isRayFacingDown && ystep < 0) ? -1 : 1;

  float nextVertTouchX = xintercept;
  float nextVertTouchY = yintercept;

  // Increment xstep and ystep until we find a wall
  while (nextVertTouchX >= 0 && nextVertTouchX <= WINDOW_WIDTH && nextVertTouchY >= 0 &&
         nextVertTouchY <= WINDOW_HEIGHT) {
    float xToCheck = nextVertTouchX + (isRayFacingLeft ? -1 : 0);
    float yToCheck = nextVertTouchY;

    if (hasWallAt(xToCheck, yToCheck)) {
      // found a wall hit
      vertWallHitX = nextVertTouchX;
      vertWallHitY = nextVertTouchY;
      vertWallContent = map[(int)floor(yToCheck / TILE_SIZE)][(int)floor(xToCheck / TILE_SIZE)];
      foundVertWallHit = TRUE;
      break;
    } else {
      nextVertTouchX += xstep;
      nextVertTouchY += ystep;
    }
  }

  // Calculate both horizontal and vertical hit distances and choose the
  // smallest one
  float horzHitDistance =
      foundHorzWallHit ? distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY) : FLT_MAX;
  float vertHitDistance =
      foundVertWallHit ? distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY) : FLT_MAX;

  if (vertHitDistance < horzHitDistance) {
    rays[stripId].distance = vertHitDistance;
    rays[stripId].wallHitX = vertWallHitX;
    rays[stripId].wallHitY = vertWallHitY;
    rays[stripId].wallHitContent = vertWallContent;
    rays[stripId].wasHitVertical = TRUE;
  } else {
    rays[stripId].distance = horzHitDistance;
    rays[stripId].wallHitX = horzWallHitX;
    rays[stripId].wallHitY = horzWallHitY;
    rays[stripId].wallHitContent = horzWallContent;
    rays[stripId].wasHitVertical = FALSE;
  }
  rays[stripId].rayAngle = rayAngle;
  rays[stripId].isRayFacingDown = isRayFacingDown;
  rays[stripId].isRayFacingUp = isRayFacingUp;
  rays[stripId].isRayFacingLeft = isRayFacingLeft;
  rays[stripId].isRayFacingRight = isRayFacingRight;
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
      SDL_FRect mapTileRect = {tileX * MINIMAP_SCALE_FACTOR, tileY * MINIMAP_SCALE_FACTOR,
                               TILE_SIZE * MINIMAP_SCALE_FACTOR, TILE_SIZE * MINIMAP_SCALE_FACTOR};

      SDL_RenderFillRectF(renderer, &mapTileRect);
    }
  }
}

void renderRays() {
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  for (int i = 0; i < NUM_RAYS; i++) {
    SDL_RenderDrawLine(renderer, player.x * MINIMAP_SCALE_FACTOR, player.y * MINIMAP_SCALE_FACTOR,
                       rays[i].wallHitX * MINIMAP_SCALE_FACTOR, rays[i].wallHitY * MINIMAP_SCALE_FACTOR);
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
  SDL_RenderDrawLine(renderer, player.x * MINIMAP_SCALE_FACTOR, player.y * MINIMAP_SCALE_FACTOR,
                     (player.x + cos(player.rotationAngle) * 40) * MINIMAP_SCALE_FACTOR,
                     (player.y + sin(player.rotationAngle) * 40) * MINIMAP_SCALE_FACTOR);
}

void generate3DWallProjection() {
  float distanceProjPlane = (WINDOW_WIDTH / 2) / tan(FOV_ANGLE / 2);
  for (int i = 0; i < NUM_RAYS; i++) {
    float fixedRaydistance = rays[i].distance * cos((rays[i].rayAngle - player.rotationAngle));

    float projectedWallHeight = TILE_SIZE / fixedRaydistance * distanceProjPlane;

    int wallStripHeight = (int)projectedWallHeight;
    int wallTopPixel = WINDOW_HEIGHT / 2 - wallStripHeight / 2;
    wallTopPixel = wallTopPixel < 0 ? 0 : wallTopPixel;

    int wallBottomPixel = WINDOW_HEIGHT / 2 + wallStripHeight / 2;
    wallBottomPixel = wallBottomPixel > WINDOW_HEIGHT ? WINDOW_HEIGHT : wallBottomPixel;

    // Paint ceeling
    uint32_t ceelingColor = 0xFFc6c58b;
    for (int y = 0; y < wallTopPixel; y++) {
      colorBuffer[(WINDOW_WIDTH * y) + i] = ceelingColor;
    }

    uint32_t textureOffsetX;
    if (rays[i].wasHitVertical) {
      textureOffsetX = (int)rays[i].wallHitY % TILE_SIZE;
    } else {
      textureOffsetX = (int)rays[i].wallHitX % TILE_SIZE;
    }

    // Paint walls with wallTexture
    for (int y = wallTopPixel; y < wallBottomPixel; y++) {
      int distanceFromTop = y + wallStripHeight / 2 - WINDOW_HEIGHT / 2;
      uint32_t textureOffsetY = distanceFromTop * ((float)TEX_HEIGHT / wallStripHeight);

      uint32_t texelColor = textures[rays[i].wallHitContent - 1][(TEX_WIDTH * textureOffsetY) + textureOffsetX];
      colorBuffer[(WINDOW_WIDTH * y) + i] = texelColor;
    }

    // Paint floor
    uint32_t floorColor = 0xFF707037;
    for (int y = wallBottomPixel; y < WINDOW_HEIGHT; y++) {
      colorBuffer[(WINDOW_WIDTH * y) + i] = floorColor;
    }
  }
}

void renderColorBuffer() {
  SDL_UpdateTexture(colorBufferTexture, NULL, colorBuffer, (int)((uint32_t)WINDOW_WIDTH * sizeof(uint32_t)));
  SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
}

void clearColorBuffer(uint32_t color) {
  for (int x = 0; x < WINDOW_WIDTH; x++) {
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
      colorBuffer[(WINDOW_WIDTH * y) + x] = color;
    }
  }
}

void renderMiniMap() {
  renderMap();
  renderRays();
  renderPlayer();
}

void renderFPS() {
  frameCount++;
  Uint32 currentTime = SDL_GetTicks();
  if (currentTime - lastTime >= 1000) {
    fps = frameCount * 1000.0f / (currentTime - lastTime);
    frameCount = 0;
    lastTime = currentTime;
  }

  char *text;
  SDL_asprintf(&text, "FPS: %d", fps);
  textSurface = TTF_RenderText_Solid(font, text, whiteColor);
  textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  SDL_Rect textRect = {WINDOW_WIDTH - textSurface->w - 5, 5, textSurface->w, textSurface->h};
  SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
  SDL_free(text);
}

void render() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);

  generate3DWallProjection();

  renderColorBuffer();
  clearColorBuffer(0xFF292929);

  renderMiniMap();
  renderFPS();

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

  lastTime = SDL_GetTicks();
  while (isGameRunning) {
    processInput();
    update();
    render();
  }

  destroyWindow();

  return 0;
}