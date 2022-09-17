package main

import "github.com/veandco/go-sdl2/sdl"

const PI = 3.14159265
const TWO_PI = 6.28318530
const TILE_SIZE = 64
const MAP_NUM_ROWS = 13
const MAP_NUM_COLS = 20
const MINIMAP_SCALE_FACTOR = 1.0
const SCREEN_WIDTH = (MAP_NUM_COLS * TILE_SIZE)
const SCREEN_HEIGHT = (MAP_NUM_ROWS * TILE_SIZE)
const FOV_ANGLE = (60 * PI / 180)
const NUM_RAYS = SCREEN_WIDTH
const FPS = 30
const FRAME_TIME_LENGTH = (1000 / FPS)

var MAP = [MAP_NUM_ROWS][MAP_NUM_COLS]int{
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
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
}

var window *sdl.Window
var renderer *sdl.Renderer
var err error
var isGameRunning bool = false
var ticksLastFrame uint64 = 0

func initializeWindow() bool {
	if err = sdl.Init(sdl.INIT_EVERYTHING); err != nil {
		println(err)
		return false
	}

	window, err = sdl.CreateWindow("test", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT, sdl.WINDOW_ALWAYS_ON_TOP)
	if err != nil {
		println(err)
		return false
	}

	renderer, err = sdl.CreateRenderer(window, -1, 0)
	if err != nil {
		println(err)
		return false
	}

	err = renderer.SetDrawBlendMode(sdl.BLENDMODE_BLEND)
	if err != nil {
		println(err)
		return false
	}

	return true
}

func destroyWindow() {
	renderer.Destroy()
	window.Destroy()
	sdl.Quit()
}

func setup() {
}

func processInput() {
	for event := sdl.PollEvent(); event != nil; event = sdl.PollEvent() {
		switch t := event.(type) {
		case *sdl.QuitEvent:
			isGameRunning = false
		case *sdl.KeyboardEvent:
			if t.Keysym.Sym == sdl.K_ESCAPE {
				isGameRunning = false
			}
		}
	}
}

func update() {
	// deltaTime := float64(sdl.GetTicks64()-ticksLastFrame) / 1000.0

	ticksLastFrame = sdl.GetTicks64()
}

func renderMap() {
	for i := 0; i < MAP_NUM_ROWS; i++ {
		for j := 0; j < MAP_NUM_COLS; j++ {
			tileX := j * TILE_SIZE
			tileY := i * TILE_SIZE
			var tileColor uint8
			if MAP[i][j] != 0 {
				tileColor = 255
			} else {
				tileColor = 0
			}

			renderer.SetDrawColor(tileColor, tileColor, tileColor, 255)
			mapTileRect := sdl.Rect{
				X: int32(tileX * MINIMAP_SCALE_FACTOR),
				Y: int32(tileY * MINIMAP_SCALE_FACTOR),
				W: TILE_SIZE * MINIMAP_SCALE_FACTOR,
				H: TILE_SIZE * MINIMAP_SCALE_FACTOR,
			}

			renderer.FillRect(&mapTileRect)
		}
	}
}

func renderRays()   {}
func renderPlayer() {}

func render() {
	renderer.SetDrawColor(0, 0, 0, 255)
	renderer.Clear()

	renderMap()
	renderRays()
	renderPlayer()

	renderer.Present()
}

func main() {
	isGameRunning = initializeWindow()

	setup()

	for isGameRunning {
		processInput()
		update()
		render()
	}

	destroyWindow()
}
