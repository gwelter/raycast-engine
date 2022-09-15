package main

import "github.com/veandco/go-sdl2/sdl"

const SCREEN_WIDTH = 1280
const SCREEN_HEIGHT = 720

var window *sdl.Window
var renderer *sdl.Renderer
var err error
var isGameRunning bool = false
var playerX, playerY int32

func initializeWindow() bool {
	if err = sdl.Init(sdl.INIT_EVERYTHING); err != nil {
		println(err)
		return false
	}

	window, err = sdl.CreateWindow("test", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT, sdl.WINDOW_BORDERLESS)
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
	playerX = 0
	playerY = 0
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
	playerX += 1
	playerY += 1
}

func render() {
	renderer.SetDrawColor(0, 0, 0, 255)
	renderer.Clear()

	renderer.SetDrawColor(255, 255, 0, 255)
	rect := sdl.Rect{X: playerX, Y: playerY, W: 20, H: 20}
	renderer.FillRect(&rect)

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