ifdef OS
  INCLUDE = -IC:/msys64/mingw64/include/SDL2 -Dmain=SDL_main -lmingw32 -mwindows -lSDL2main -lSDL2
else
	INCLUDE = -lSDL2main -lSDL2 -lm
endif

build:
	gcc -std=c99 ./src/*.c -o raycast $(INCLUDE)

run:
	./raycast;

clean:
	rm raycast;