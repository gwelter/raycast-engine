pub const PI = 3.14159265359;
pub const TWO_PI = 6.28318530718;

pub const TILE_SIZE = 64;
pub const MAP_NUM_ROWS = 13;
pub const MAP_NUM_COLS = 20;
pub const MINIMAP_SCALE_FACTOR = 0.2;

pub const SCREEN_WIDTH = (MAP_NUM_COLS * TILE_SIZE);
pub const SCREEN_HEIGHT = (MAP_NUM_ROWS * TILE_SIZE);
pub const WINDOW_WIDTH = SCREEN_WIDTH;
pub const WINDOW_HEIGHT = SCREEN_HEIGHT;

pub const TEX_WIDTH = 64;
pub const TEX_HEIGHT = 64;
pub const NUM_TEXTURES = 8;

pub const FOV_ANGLE = (60 * PI / 180);
pub const NUM_RAYS = SCREEN_WIDTH;

pub const FPS = 60;
pub const FRAME_DELAY = (1000 / FPS);

pub const FALSE = 0;
pub const TRUE = 1;

pub const MY_FONT = "../assets/font.ttf";
