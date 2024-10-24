CC := gcc
build:
	$(CC) main.c board.c stb.c -lSDL3 -lm -o chess && ./chess
clean:
	rm chess
