ifdef OS
  INCLUDE = -IC:/msys64/mingw64/include/SDL2 -Dmain=SDL_main -lmingw32 -mwindows -lSDL2main -lSDL2 -lSDL2_ttf
else
	INCLUDE = -lSDL2main -lSDL2 -lm -lSDL2_ttf
endif

SRC := $(filter-out ./src/main.test.c,$(wildcard ./src/*.c))

.PHONY: build run clean test

build:
	gcc -std=c99 $(SRC) -o raycast $(INCLUDE)

run:
	./raycast;

clean:
	rm -f raycast raycast_tests;

test:
	# Build unit tests (requires: libcmocka-dev, libsdl2-dev, libsdl2-ttf-dev)
	gcc -std=c99 -Wall -Wextra -Wno-unused-parameter -I./src ./src/main.test.c -o raycast_tests $(INCLUDE) $$(pkg-config --cflags --libs cmocka 2>/dev/null || echo -lcmocka)
	./raycast_tests | cat