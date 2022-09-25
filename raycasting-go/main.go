package main

import (
	"math"

	"github.com/veandco/go-sdl2/sdl"
)

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

type Player struct {
	x             float32
	y             float32
	width         float32
	height        float32
	rotationAngle float32
	walkSpeed     float32
	turnSpeed     float32
	turnDirection int // -1 left - 1 right
	walkDirection int // -1 for back - 1 for front
}

type Ray struct {
	rayAngle         float32
	wallHitX         float32
	wallHitY         float32
	distance         float32
	wasHitVertical   int
	wallHitContent   int
	isRayFacingUp    int
	isRayFacingDown  int
	isRayFacingLeft  int
	isRayFacingRight int
}

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
var ticksLastFrame uint32 = 0

var player Player

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
	player = Player{
		x:             SCREEN_WIDTH / 2,
		y:             SCREEN_HEIGHT / 2,
		width:         5,
		height:        5,
		turnDirection: 0,
		walkDirection: 0,
		rotationAngle: PI / 2.0,
		walkSpeed:     150,
		turnSpeed:     100 * PI / 180,
	}
}

func processInput() {
	for event := sdl.PollEvent(); event != nil; event = sdl.PollEvent() {
		switch t := event.(type) {
		case *sdl.QuitEvent:
			isGameRunning = false
		case *sdl.KeyboardEvent:
			if t.Type == sdl.KEYDOWN {
				if t.Keysym.Sym == sdl.K_ESCAPE {
					isGameRunning = false
				}
				if t.Keysym.Sym == sdl.K_UP {
					player.walkDirection = 1
				}
				if t.Keysym.Sym == sdl.K_DOWN {
					player.walkDirection = -1
				}
				if t.Keysym.Sym == sdl.K_RIGHT {
					player.turnDirection = 1
				}
				if t.Keysym.Sym == sdl.K_LEFT {
					player.turnDirection = -1
				}
			}
			if t.Type == sdl.KEYUP {
				if t.Keysym.Sym == sdl.K_UP {
					player.walkDirection = 0
				}
				if t.Keysym.Sym == sdl.K_DOWN {
					player.walkDirection = 0
				}
				if t.Keysym.Sym == sdl.K_RIGHT {
					player.turnDirection = 0
				}
				if t.Keysym.Sym == sdl.K_LEFT {
					player.turnDirection = 0
				}
			}
		}
	}
}

func hasWallAt(x, y float32) bool {
	if x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT {
		return true
	}

	i := int(math.Floor(float64(y / TILE_SIZE)))
	j := int(math.Floor(float64(x / TILE_SIZE)))
	return MAP[i][j] != 0
}

func movePlayer(deltaTime float32) {
	player.rotationAngle += float32(player.turnDirection) * player.turnSpeed * deltaTime
	moveSpeed := player.walkSpeed * float32(player.walkDirection) * deltaTime

	nextX := player.x + float32(math.Cos(float64(player.rotationAngle)))*moveSpeed
	nextY := player.y + float32(math.Sin(float64(player.rotationAngle)))*moveSpeed

	if hasWallAt(nextX, nextY) {
		return
	}

	player.x = nextX
	player.y = nextY
}

func castRay(rayAngle float32, stripId int) {

}

func castAllRays() {
	// start first ray subtracting half of the FOV
	rayAngle := player.rotationAngle - (FOV_ANGLE / 2)
	for stripId := 0; stripId < NUM_RAYS; stripId++ {
		castRay(rayAngle, stripId)

		rayAngle += FOV_ANGLE / NUM_RAYS
	}
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

func renderRays() {}

func renderPlayer() {
	renderer.SetDrawColor(255, 255, 255, 255)
	playerRect := sdl.Rect{
		X: int32(player.x),
		Y: int32(player.y),
		W: int32(player.width),
		H: int32(player.height),
	}
	renderer.FillRect(&playerRect)

	endOfLineX := player.x + float32(math.Cos(float64(player.rotationAngle)))*40*MINIMAP_SCALE_FACTOR
	endOfLineY := player.y + float32(math.Sin(float64(player.rotationAngle)))*40*MINIMAP_SCALE_FACTOR
	renderer.DrawLine(
		int32(player.x)*MINIMAP_SCALE_FACTOR,
		int32(player.y)*MINIMAP_SCALE_FACTOR,
		int32(endOfLineX),
		int32(endOfLineY),
	)
}

func render() {
	renderer.SetDrawColor(0, 0, 0, 255)
	renderer.Clear()

	renderMap()
	renderRays()
	renderPlayer()

	renderer.Present()
}

func update() {
	deltaTime := float32(sdl.GetTicks()-ticksLastFrame) / 1000.0

	movePlayer(deltaTime)
	castAllRays()

	ticksLastFrame = sdl.GetTicks()
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
