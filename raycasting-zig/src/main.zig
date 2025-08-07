const constants = @import("constants.zig");
const std = @import("std");
const c = @cImport({
    @cInclude("SDL2/SDL.h");
    @cInclude("SDL2/SDL_ttf.h");
});

const whiteColor = c.SDL_Color{ .r = 255, .g = 255, .b = 255 };
const map = [constants.MAP_NUM_ROWS][constants.MAP_NUM_COLS]i32{
    i32{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 1 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    i32{ 1, 0, 0, 0, 2, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 0, 0, 0, 0, 1 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 1 },
    i32{ 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5 },
    i32{ 1, 0, 0, 0, 7, 3, 6, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5 },
    i32{ 1, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 5 },
    i32{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 5, 5, 5, 5, 5 },
};

var isGameRunning: bool = false;
var window: ?*c.SDL_Window = null;
var renderer: ?*c.SDL_Renderer = null;
var font: ?*c.TTF_Font = null;
var textSurface: ?*c.SDL_Surface = null;
var textTexture: ?*c.SDL_Texture = null;

var ticksLastFrame: u32 = 0;
var lastTime: u32 = 0;
var frameCount: u32 = 0;
var fps: u32 = 0;

fn initializeWindow() !bool {
    if (c.SDL_Init(c.SDL_INIT_VIDEO) != 0) {
        std.debug.print("SDL initialization failed\n", .{});
        return false;
    }

    window = c.SDL_CreateWindow(
        "Raycasting",
        c.SDL_WINDOWPOS_CENTERED,
        c.SDL_WINDOWPOS_CENTERED,
        constants.WINDOW_WIDTH,
        constants.WINDOW_HEIGHT,
        c.SDL_WINDOW_BORDERLESS,
    );
    if (window == null) {
        std.debug.print("Window creation failed: {s}\n", .{c.SDL_GetError()});
        return false;
    }

    renderer = c.SDL_CreateRenderer(window.?, -1, 0);
    if (renderer == null) {
        std.debug.print("Renderer creation failed\n", .{});
        return false;
    }

    _ = c.SDL_SetRenderDrawBlendMode(renderer.?, c.SDL_BLENDMODE_BLEND);
    if (c.TTF_Init() != 0) {
        std.debug.print("TTF initialization failed: {s}\n", .{c.TTF_GetError()});
        return false;
    }
    font = c.TTF_OpenFont(constants.MY_FONT, 24);
    if (font == null) {
        std.debug.print("Font loading failed: {s}\n", .{c.TTF_GetError()});
        return false;
    }

    return true;
}

fn setup() void {
    // Initialize the game state, load resources, etc.
}

fn destroyWindow() void {
    // Clean up resources, close the window, etc.
    c.SDL_DestroyRenderer(renderer);
    c.SDL_DestroyWindow(window);
    c.SDL_Quit();
}

fn processInput() void {
    var event: c.SDL_Event = undefined;
    _ = c.SDL_PollEvent(&event);
    switch (event.type) {
        c.SDL_QUIT => isGameRunning = false,
        c.SDL_KEYDOWN => switch (event.key.keysym.sym) {
            c.SDLK_ESCAPE => isGameRunning = false,
            else => {},
        },
        else => {},
    }
}

fn update() void {
    const frameRate: u32 = (c.SDL_GetTicks() % ticksLastFrame);
    if (frameRate > constants.FRAME_DELAY) {
        ticksLastFrame = c.SDL_GetTicks();
        return;
    }
    const timeToWait = constants.FRAME_DELAY - frameRate;
    if (timeToWait > 0 and timeToWait < constants.FRAME_DELAY) {
        c.SDL_Delay(timeToWait);
    }

    ticksLastFrame = c.SDL_GetTicks();
}

fn render_fps() void {
    frameCount += 1;
    const currentTime = c.SDL_GetTicks();
    if (currentTime - lastTime >= 1000) {
        fps = frameCount * 1000;
        frameCount = 0;
        lastTime = currentTime;
    }

    const text = std.fmt.allocPrint(
        std.heap.page_allocator,
        "FPS: {d}",
        .{fps},
    ) catch {
        std.debug.print("Failed to allocate memory for FPS text\n", .{});
        return;
    };
    defer std.heap.page_allocator.free(text);

    textSurface = c.TTF_RenderText_Solid(font.?, text.ptr, whiteColor);
    textTexture = c.SDL_CreateTextureFromSurface(renderer.?, textSurface.?);
    const textRect = c.SDL_Rect{
        .x = 5,
        .y = 5,
        .w = textSurface.?.w,
        .h = textSurface.?.h,
    };
    _ = c.SDL_RenderCopy(renderer.?, textTexture.?, null, &textRect);
}

fn render() void {
    _ = c.SDL_SetRenderDrawColor(renderer.?, 0, 0, 0, 255);
    _ = c.SDL_RenderClear(renderer.?);
    render_fps();

    _ = c.SDL_RenderPresent(renderer.?);
}

pub fn main() !void {
    isGameRunning = initializeWindow() catch {
        std.debug.print("Failed to initialize window\n", .{});
        return;
    };

    setup();
    ticksLastFrame = c.SDL_GetTicks();
    while (isGameRunning) {
        processInput();
        update();
        render();
    }

    destroyWindow();
    return;
}
