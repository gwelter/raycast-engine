# Detect OS
UNAME_S := $(shell uname -s)

ifeq ($(OS),Windows_NT)
    # Windows (MSYS2/MinGW)
    INCLUDE = -IC:/msys64/mingw64/include/SDL2 -Dmain=SDL_main \
              -lmingw32 -mwindows -lSDL2main -lSDL2 -lSDL2_ttf
else ifeq ($(UNAME_S),Darwin)
    # macOS (Homebrew) → use pkg-config
    INCLUDE = $(shell pkg-config --cflags --libs sdl2 SDL2_ttf)
else
    # Linux
    INCLUDE = -lSDL2main -lSDL2 -lm -lSDL2_ttf
endif

SRC := $(filter-out ./src/main.test.c,$(wildcard ./src/*.c))

.PHONY: build run clean test

build:
	gcc -std=c99 $(SRC) -o raycast $(INCLUDE)

run: build
	./raycast

clean:
	rm -f raycast raycast_tests

test:
	# Build unit tests (requires: cmocka + SDL2 + SDL2_ttf)
	gcc -std=c99 -Wall -Wextra -Wno-unused-parameter -I./src \
	    ./src/main.test.c -o raycast_tests \
	    $(INCLUDE) \
	    $$(pkg-config --cflags --libs cmocka 2>/dev/null || echo -lcmocka)
	./raycast_tests | cat
