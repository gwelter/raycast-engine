ifdef OS
  INCLUDE = -Isrc/Include -Lsrc/lib -lmingw32mak
endif

build:
	gcc -std=c99 ./src/*.c $(INCLUDE) -lSDL2main -lSDL2 -o raycast

run:
	./raycast;

clean:
	rm raycast;