package main

import (
	"fmt"
	"math"
	"unsafe"

	"github.com/veandco/go-sdl2/sdl"
	"github.com/veandco/go-sdl2/ttf"
)

const PI = math.Pi
const TWO_PI = math.Pi * 2
const TILE_SIZE = 64
const MAP_NUM_ROWS = 13
const MAP_NUM_COLS = 20
const MINIMAP_SCALE_FACTOR = 0.2
const MINIMAP_TILE_SIZE = 12
const WINDOW_WIDTH = SCREEN_WIDTH
const WINDOW_HEIGHT = SCREEN_HEIGHT
const TEX_WIDTH = 64
const TEX_HEIGHT = 64
const NUM_TEXTURES = 8
const MY_FONT = "../assets/font.ttf"
const SCREEN_WIDTH = (MAP_NUM_COLS * TILE_SIZE)
const SCREEN_HEIGHT = (MAP_NUM_ROWS * TILE_SIZE)
const FOV_ANGLE = (60 * PI / 180)
const NUM_RAYS = SCREEN_WIDTH
const FPS = 60
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
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 2, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 9, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
	{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5},
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5},
}

var window *sdl.Window
var renderer *sdl.Renderer
var err error
var isGameRunning bool = false
var ticksLastFrame uint64 = 0

var player Player
var rays [NUM_RAYS]Ray

var colorBuffer []uint32
var colorBufferTexture *sdl.Texture
var font *ttf.Font
var lastTime uint64
var frameCount int = 0
var fps int = 0

// Basic texture data - simplified colored textures
var textures [NUM_TEXTURES][TEX_WIDTH * TEX_HEIGHT]uint32

func initializeWindow() bool {
	if err = sdl.Init(sdl.INIT_VIDEO); err != nil {
		fmt.Printf("Error initializing SDL: %v\n", err)
		return false
	}

	window, err = sdl.CreateWindow("Raycasting", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT, sdl.WINDOW_BORDERLESS)
	if err != nil {
		fmt.Printf("Error creating window: %v\n", err)
		return false
	}

	renderer, err = sdl.CreateRenderer(window, -1, 0)
	if err != nil {
		fmt.Printf("Error creating renderer: %v\n", err)
		return false
	}

	err = renderer.SetDrawBlendMode(sdl.BLENDMODE_BLEND)
	if err != nil {
		fmt.Printf("Error setting blend mode: %v\n", err)
		return false
	}

	if err = ttf.Init(); err != nil {
		fmt.Printf("Error initializing TTF: %v\n", err)
		return false
	}
	font, err = ttf.OpenFont(MY_FONT, 24)
	if err != nil {
		fmt.Printf("Error opening font: %v\n", err)
		return false
	}

	return true
}

func destroyWindow() {
	if colorBufferTexture != nil {
		colorBufferTexture.Destroy()
	}
	if font != nil {
		font.Close()
	}
	ttf.Quit()
	renderer.Destroy()
	window.Destroy()
	sdl.Quit()
}

func setup() {
	player = Player{
		x:             SCREEN_WIDTH / 2,
		y:             SCREEN_HEIGHT / 2,
		width:         1,
		height:        1,
		turnDirection: 0,
		walkDirection: 0,
		rotationAngle: PI / 2.0,
		walkSpeed:     150,
		turnSpeed:     100 * PI / 180,
	}

	colorBuffer = make([]uint32, WINDOW_WIDTH*WINDOW_HEIGHT)
	colorBufferTexture, err = renderer.CreateTexture(sdl.PIXELFORMAT_ARGB8888, sdl.TEXTUREACCESS_STREAMING, WINDOW_WIDTH, WINDOW_HEIGHT)
	if err != nil {
		fmt.Printf("Error creating color buffer texture: %v\n", err)
	}

	initTextures()
}

func initTextures() {
	// Simple colored textures for now - you can replace with real texture data later
	colors := []uint32{
		0xFF8B4513, // REDBRICK - brown
		0xFF800080, // PURPLESTONE - purple
		0xFF556B2F, // MOSSYSTONE - olive
		0xFF808080, // GRAYSTONE - gray
		0xFFFF6347, // COLORSTONE - tomato
		0xFF4169E1, // BLUESTONE - royal blue
		0xFFD2691E, // WOOD - chocolate
		0xFF8B4513, // EAGLE - brown
	}

	for i := 0; i < NUM_TEXTURES; i++ {
		for j := 0; j < TEX_WIDTH*TEX_HEIGHT; j++ {
			textures[i][j] = colors[i]
		}
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

func castRay(rayAngle float64, stripId int) {
	rayAngle = normalizeAngle(rayAngle)
	isRayFacingDown := rayAngle > 0 && rayAngle < PI
	isRayFacingUp := !isRayFacingDown

	isRayFacingRight := rayAngle < 0.5*PI || rayAngle > 1.5*PI
	isRayFacingLeft := !isRayFacingRight

	///////////////////////////////////////////
	// HORIZONTAL RAY-GRID INTERSECTION CODE
	///////////////////////////////////////////
	foundHorzWallHit := false
	horzWallHitX := 0.0
	horzWallHitY := 0.0
	horzWallContent := 0

	// Find the y-coordinate of the closest horizontal grid intersection
	var yintercept float64 = math.Floor(player.y/TILE_SIZE) * TILE_SIZE
	if isRayFacingDown {
		yintercept += TILE_SIZE
	}

	// Find the x-coordinate of the closest horizontal grid intersection
	var xintercept float64 = player.x + (yintercept-player.y)/math.Tan(rayAngle)

	// Calculate the increment xstep and ystep
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

	nextHorzTouchX := xintercept
	nextHorzTouchY := yintercept

	// Increment xstep and ystep until we find a wall
	for nextHorzTouchX >= 0 && nextHorzTouchX <= SCREEN_WIDTH && nextHorzTouchY >= 0 && nextHorzTouchY <= SCREEN_HEIGHT {
		xToCheck := nextHorzTouchX
		yToCheck := nextHorzTouchY
		if isRayFacingUp {
			yToCheck += -1
		}

		if hasWallAt(xToCheck, yToCheck) {
			// found a wall hit
			horzWallHitX = nextHorzTouchX
			horzWallHitY = nextHorzTouchY
			horzWallContent = wallContentAt(xToCheck, yToCheck)
			foundHorzWallHit = true
			break
		} else {
			nextHorzTouchX += xstep
			nextHorzTouchY += float64(ystep)
		}
	}

	///////////////////////////////////////////
	// VERTICAL RAY-GRID INTERSECTION CODE
	///////////////////////////////////////////
	foundVertWallHit := false
	vertWallHitX := 0.0
	vertWallHitY := 0.0
	vertWallContent := 0

	// Find the x-coordinate of the closest horizontal grid intersection
	xintercept = math.Floor(player.x/TILE_SIZE) * TILE_SIZE
	if isRayFacingRight {
		xintercept += float64(TILE_SIZE)
	}

	// Find the y-coordinate of the closest horizontal grid intersection
	yintercept = player.y + (xintercept-player.x)*math.Tan(rayAngle)

	// Calculate the increment xstep and ystep
	xstep = TILE_SIZE
	if isRayFacingLeft {
		xstep *= -1
	}

	ystep = TILE_SIZE * math.Tan(rayAngle)
	if isRayFacingUp && ystep > 0 {
		ystep *= -1
	}
	if isRayFacingDown && ystep < 0 {
		ystep *= -1
	}

	nextVertTouchX := xintercept
	nextVertTouchY := yintercept

	// Increment xstep and ystep until we find a wall
	for nextVertTouchX >= 0 && nextVertTouchX <= SCREEN_WIDTH && nextVertTouchY >= 0 && nextVertTouchY <= SCREEN_HEIGHT {
		xToCheck := nextVertTouchX
		if isRayFacingLeft {
			xToCheck += -1
		}
		yToCheck := nextVertTouchY

		if hasWallAt(xToCheck, yToCheck) {
			// found a wall hit
			vertWallHitX = nextVertTouchX
			vertWallHitY = nextVertTouchY
			vertWallContent = wallContentAt(xToCheck, yToCheck)
			foundVertWallHit = true
			break
		} else {
			nextVertTouchX += xstep
			nextVertTouchY += ystep
		}
	}

	// Calculate both horizontal and vertical hit distances and choose the smallest one
	horzHitDistance := math.MaxFloat64
	if foundHorzWallHit {
		horzHitDistance = distanceBetweenPoints(player.x, player.y, horzWallHitX, horzWallHitY)
	}
	vertHitDistance := math.MaxFloat64
	if foundVertWallHit {
		vertHitDistance = distanceBetweenPoints(player.x, player.y, vertWallHitX, vertWallHitY)
	}

	if vertHitDistance < horzHitDistance {
		rays[stripId].distance = vertHitDistance
		rays[stripId].wallHitX = vertWallHitX
		rays[stripId].wallHitY = vertWallHitY
		rays[stripId].wallHitContent = vertWallContent
		rays[stripId].wasHitVertical = true
	} else {
		rays[stripId].distance = horzHitDistance
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
				X: int32(float64(tileX) * MINIMAP_SCALE_FACTOR),
				Y: int32(float64(tileY) * MINIMAP_SCALE_FACTOR),
				W: int32(MINIMAP_TILE_SIZE),
				H: int32(MINIMAP_TILE_SIZE),
			}

			renderer.FillRect(&mapTileRect)
		}
	}
}

func renderRays() {
	renderer.SetDrawColor(255, 0, 0, 255)
	for i := 0; i < NUM_RAYS; i++ {
		renderer.DrawLine(
			int32(player.x*MINIMAP_SCALE_FACTOR),
			int32(player.y*MINIMAP_SCALE_FACTOR),
			int32(rays[i].wallHitX*MINIMAP_SCALE_FACTOR),
			int32(rays[i].wallHitY*MINIMAP_SCALE_FACTOR),
		)
	}
}

func renderPlayer() {
	renderer.SetDrawColor(255, 255, 255, 255)

	endOfLineX := (player.x + math.Cos(player.rotationAngle)*40) * MINIMAP_SCALE_FACTOR
	endOfLineY := (player.y + math.Sin(player.rotationAngle)*40) * MINIMAP_SCALE_FACTOR
	renderer.DrawLine(
		int32(player.x*MINIMAP_SCALE_FACTOR),
		int32(player.y*MINIMAP_SCALE_FACTOR),
		int32(endOfLineX),
		int32(endOfLineY),
	)
}

func generate3DWallProjection() {
	distanceProjPlane := float64(WINDOW_WIDTH/2) / math.Tan(FOV_ANGLE/2)
	for i := 0; i < NUM_RAYS; i++ {
		fixedRayDistance := rays[i].distance * math.Cos(rays[i].rayAngle-player.rotationAngle)

		projectedWallHeight := float64(TILE_SIZE) / fixedRayDistance * distanceProjPlane

		wallStripHeight := int(projectedWallHeight)
		wallTopPixel := WINDOW_HEIGHT/2 - wallStripHeight/2
		if wallTopPixel < 0 {
			wallTopPixel = 0
		}

		wallBottomPixel := WINDOW_HEIGHT/2 + wallStripHeight/2
		if wallBottomPixel > WINDOW_HEIGHT {
			wallBottomPixel = WINDOW_HEIGHT
		}

		// Paint ceiling
		ceilingColor := uint32(0xFFc6c58b)
		for y := 0; y < wallTopPixel; y++ {
			colorBuffer[WINDOW_WIDTH*y+i] = ceilingColor
		}

		// Calculate texture offset
		var textureOffsetX int
		if rays[i].wasHitVertical {
			textureOffsetX = int(rays[i].wallHitY) % TILE_SIZE
		} else {
			textureOffsetX = int(rays[i].wallHitX) % TILE_SIZE
		}

		// Paint walls with texture
		for y := wallTopPixel; y < wallBottomPixel; y++ {
			distanceFromTop := y + wallStripHeight/2 - WINDOW_HEIGHT/2
			textureOffsetY := int(float64(distanceFromTop) * float64(TEX_HEIGHT) / float64(wallStripHeight))

			if textureOffsetY >= 0 && textureOffsetY < TEX_HEIGHT && textureOffsetX >= 0 && textureOffsetX < TEX_WIDTH && rays[i].wallHitContent > 0 && rays[i].wallHitContent <= NUM_TEXTURES {
				texelColor := textures[rays[i].wallHitContent-1][TEX_WIDTH*textureOffsetY+textureOffsetX]
				colorBuffer[WINDOW_WIDTH*y+i] = texelColor
			}
		}

		// Paint floor
		floorColor := uint32(0xFF707037)
		for y := wallBottomPixel; y < WINDOW_HEIGHT; y++ {
			colorBuffer[WINDOW_WIDTH*y+i] = floorColor
		}
	}
}

func renderColorBuffer() {
	colorBufferTexture.Update(nil, unsafe.Pointer(&colorBuffer[0]), WINDOW_WIDTH*4)
	renderer.Copy(colorBufferTexture, nil, nil)
}

func clearColorBuffer(color uint32) {
	for i := 0; i < len(colorBuffer); i++ {
		colorBuffer[i] = color
	}
}

func renderFPS() {
	frameCount++
	currentTime := sdl.GetTicks64()
	if currentTime-lastTime >= 1000 {
		fps = int(float64(frameCount) * 1000.0 / float64(currentTime-lastTime))
		frameCount = 0
		lastTime = currentTime
	}

	text := fmt.Sprintf("FPS: %d", fps)
	surface, err := font.RenderUTF8Solid(text, sdl.Color{R: 255, G: 255, B: 255, A: 255})
	if err == nil {
		textTexture, err := renderer.CreateTextureFromSurface(surface)
		if err == nil {
			textRect := sdl.Rect{
				X: int32(WINDOW_WIDTH - int(surface.W) - 5),
				Y: 5,
				W: surface.W,
				H: surface.H,
			}
			renderer.Copy(textTexture, nil, &textRect)
			textTexture.Destroy()
		}
		surface.Free()
	}
}

func renderMiniMap() {
	renderMap()
	renderRays()
	renderPlayer()
}

func render() {
	renderer.SetDrawColor(0, 0, 0, 255)
	renderer.Clear()

	generate3DWallProjection()

	renderColorBuffer()
	clearColorBuffer(0xFF292929)

	renderMiniMap()
	renderFPS()

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

	lastTime = sdl.GetTicks64()
	for isGameRunning {
		processInput()
		update()
		render()
	}

	destroyWindow()
}
