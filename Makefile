CC := gcc
build:
	$(CC) main.c board.c texture.c -lSDL3 -lm -o chess && ./chess
clean:
	rm chess
