pong : pong.c
	gcc -o pong -I/usr/include/SDL2 pong.c -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2 -lpthread

clear:
	rm -f pong
