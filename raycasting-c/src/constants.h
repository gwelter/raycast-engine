#define PI 3.14159265359
#define TWO_PI 6.28318530718

#define TILE_SIZE 64
#define MAP_NUM_ROWS 13
#define MAP_NUM_COLS 20
#define MINIMAP_SCALE_FACTOR 1.0

#define SCREEN_WIDTH (MAP_NUM_COLS * TILE_SIZE)
#define SCREEN_HEIGHT (MAP_NUM_ROWS * TILE_SIZE)

#define FOV_ANGLE (60 * PI / 180)
#define NUM_RAYS SCREEN_WIDTH

#define FPS 30
#define FRAME_TIME_LENGTH (1000 / FPS)

#define FALSE 0
#define TRUE 1