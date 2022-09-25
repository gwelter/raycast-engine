package main

import (
	"math"

	"github.com/veandco/go-sdl2/sdl"
)

const PI = math.Pi
const TWO_PI = math.Pi * 2
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
	x             float64
	y             float64
	width         float64
	height        float64
	rotationAngle float64
	walkSpeed     float64
	turnSpeed     float64
	turnDirection int // -1 left - 1 right
	walkDirection int // -1 for back - 1 for front
}

type Ray struct {
	rayAngle         float64
	wallHitX         float64
	wallHitY         float64
	distance         float64
	wallHitContent   int
	wasHitVertical   bool
	isRayFacingUp    bool
	isRayFacingDown  bool
	isRayFacingLeft  bool
	isRayFacingRight bool
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
var ticksLastFrame uint64 = 0

var player Player
var rays [NUM_RAYS]Ray

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

func hasWallAt(x, y float64) bool {
	if x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT {
		return true
	}

	i := int(math.Floor(y / TILE_SIZE))
	j := int(math.Floor(x / TILE_SIZE))
	return MAP[i][j] != 0
}

func wallContentAt(x, y float64) int {
	if x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT {
		return 0
	}

	i := int(math.Floor(y / TILE_SIZE))
	j := int(math.Floor(x / TILE_SIZE))
	return MAP[i][j]
}

func movePlayer(deltaTime float64) {
	player.rotationAngle += float64(player.turnDirection) * player.turnSpeed * deltaTime
	moveSpeed := player.walkSpeed * float64(player.walkDirection) * deltaTime

	nextX := player.x + math.Cos(player.rotationAngle)*moveSpeed
	nextY := player.y + math.Sin(player.rotationAngle)*moveSpeed

	if hasWallAt(nextX, nextY) {
		return
	}

	player.x = nextX
	player.y = nextY
}

func distanceBetweenPoints(x1, y1, x2, y2 float64) float64 {
	x := (x2 - x1) * (x2 - x1)
	y := (y2 - y1) * (y2 - y1)
	return math.Sqrt(x + y)
}

func normalizeAngle(angle float64) float64 {
	angle = math.Mod(angle, TWO_PI)
	if angle < 0 {
		angle = TWO_PI + angle
	}
	return angle
}

func horizontalInterception(rayAngle float64, isRayFacingDown bool) (float64, float64) {
	yintercept := math.Floor(player.y/TILE_SIZE) * TILE_SIZE
	if isRayFacingDown {
		yintercept += TILE_SIZE
	}

	opositeSide := yintercept - player.y

	xintercept := player.x + opositeSide/math.Tan(rayAngle)

	return xintercept, yintercept
}

func horizontalStep(rayAngle, xintercept, yintercept float64, isRayFacingUp, isRayFacingLeft, isRayFacingRight bool) (float64, float64, int) {
	wallContent := 0
	nextXTouch := xintercept
	nextYTouch := yintercept

	ystep := float64(TILE_SIZE)
	if isRayFacingUp {
		ystep *= -1
	}

	xstep := TILE_SIZE / math.Tan(rayAngle)
	if isRayFacingLeft && xstep > 0 {
		xstep *= -1
	}
	if isRayFacingRight && xstep < 0 {
		xstep *= -1
	}

	for {
		xToCheck := nextXTouch
		yToCheck := nextYTouch
		if isRayFacingUp {
			yToCheck -= 1
		}
		if hasWallAt(xToCheck, yToCheck) {
			wallContent = wallContentAt(xToCheck, yToCheck)
			break
		} else {
			nextXTouch += xstep
			nextYTouch += ystep
		}
	}

	return nextXTouch, nextYTouch, wallContent
}
func verticalInterception(rayAngle float64, isRayFacingRight bool) (float64, float64) {
	xintercept := math.Floor(player.x/TILE_SIZE) * TILE_SIZE
	if isRayFacingRight {
		xintercept += TILE_SIZE
	}

	adjecentSide := xintercept - player.x

	yintercept := player.y + adjecentSide*math.Tan(rayAngle)
	return xintercept, yintercept
}

func verticalStep(rayAngle, xintercept, yintercept float64, isRayFacingUp, isRayFacingDown, isRayFacingLeft bool) (float64, float64, int) {
	wallContent := 0
	nextXTouch := xintercept
	nextYTouch := yintercept

	xstep := float64(TILE_SIZE)
	if isRayFacingLeft {
		xstep *= -1
	}

	ystep := TILE_SIZE * math.Tan(rayAngle)
	if isRayFacingUp && ystep > 0 {
		ystep *= -1
	}
	if isRayFacingDown && ystep < 0 {
		ystep *= -1
	}

	for {
		xToCheck := nextXTouch
		if isRayFacingLeft {
			xToCheck -= 1
		}
		yToCheck := nextYTouch
		if hasWallAt(xToCheck, yToCheck) {
			wallContent = wallContentAt(xToCheck, yToCheck)
			break
		} else {
			nextXTouch += xstep
			nextYTouch += ystep
		}
	}
	return nextXTouch, nextYTouch, wallContent
}

func castRay(rayAngle float64, stripId int) {
	rayAngle = normalizeAngle(rayAngle)
	isRayFacingDown := rayAngle > 0 && rayAngle < PI
	isRayFacingUp := !isRayFacingDown
	isRayFacingLeft := rayAngle < (0.5*PI) || rayAngle > (1.5*PI)
	isRayFacingRight := !isRayFacingLeft

	xintercept, yintercept := horizontalInterception(rayAngle, isRayFacingDown)
	horzWallHitX, horzWallHitY, horzWallContent := horizontalStep(rayAngle, xintercept, yintercept, isRayFacingUp, isRayFacingLeft, isRayFacingDown)

	xintercept, yintercept = verticalInterception(rayAngle, isRayFacingRight)
	vertWallHitX, vertWallHitY, vertWallContent := verticalStep(rayAngle, xintercept, yintercept, isRayFacingUp, isRayFacingDown, isRayFacingLeft)

	horizontalDistance := distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
	verticalDistance := distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)

	if verticalDistance < horizontalDistance {
		rays[stripId].distance = verticalDistance
		rays[stripId].wallHitX = vertWallHitX
		rays[stripId].wallHitY = vertWallHitY
		rays[stripId].wallHitContent = vertWallContent
		rays[stripId].wasHitVertical = true
	} else {
		rays[stripId].distance = horizontalDistance
		rays[stripId].wallHitX = horzWallHitX
		rays[stripId].wallHitY = horzWallHitY
		rays[stripId].wallHitContent = horzWallContent
		rays[stripId].wasHitVertical = false
	}
	rays[stripId].rayAngle = rayAngle
	rays[stripId].isRayFacingDown = isRayFacingDown
	rays[stripId].isRayFacingUp = isRayFacingUp
	rays[stripId].isRayFacingLeft = isRayFacingLeft
	rays[stripId].isRayFacingRight = isRayFacingRight
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

func renderRays() {
	renderer.SetDrawColor(255, 0, 0, 255)
	for i := 0; i < NUM_RAYS; i++ {
		renderer.DrawLine(
			int32(player.x)*MINIMAP_SCALE_FACTOR,
			int32(player.y)*MINIMAP_SCALE_FACTOR,
			int32(rays[i].wallHitX*MINIMAP_SCALE_FACTOR),
			int32(rays[i].wallHitY*MINIMAP_SCALE_FACTOR),
		)
	}
}

func renderPlayer() {
	renderer.SetDrawColor(255, 255, 255, 255)
	playerRect := sdl.Rect{
		X: int32(player.x),
		Y: int32(player.y),
		W: int32(player.width),
		H: int32(player.height),
	}
	renderer.FillRect(&playerRect)

	endOfLineX := player.x + math.Cos(player.rotationAngle)*40*MINIMAP_SCALE_FACTOR
	endOfLineY := player.y + math.Sin(player.rotationAngle)*40*MINIMAP_SCALE_FACTOR
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
	deltaTime := float64(sdl.GetTicks64()-ticksLastFrame) / 1000.0

	movePlayer(deltaTime)
	castAllRays()

	ticksLastFrame = sdl.GetTicks64()
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
