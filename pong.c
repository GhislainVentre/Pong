/*****************************************************************************************
 * 	Copyright (c) 2022 VENTRE Ghislain
 * 	All rights reserved.
 * **************************************************************************************/

#include <SDL2/SDL.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void render_menu();
void render_options();
void render_score();
void render_winner();
void render_onlinemenu();

void event_handler();
void update_ball();

void init();
void init_players();
void init_ball();
void init_game();

struct game {
    int score1, score2;
    int goal;
    enum game_state {
		MENU,
		OPTIONS,
		GAME_MENU,
        ON,
		ONLINE,
        END,
        QUIT
    } state;
} game;

struct ball {
    int ix, iy;
    int vx, vy;
    int bounced, bounced_tot;
} ballparam;

// screen size
int width, height;

// SDL Window
SDL_Window *window;
SDL_Renderer *renderer;

// Sound effects
Mix_Chunk *sound_bounce;
Mix_Chunk *sound_menu;
Mix_Chunk *sound_point;
Mix_Chunk *sound_endgame;
Mix_Chunk *sound_bounce_wall;

// Ball
SDL_Rect ball;
int ispeed = 10;

// Players
SDL_Rect player1;
SDL_Rect player2;
int pspeed;
int hrect;
int wrect;

int main(int argc, char ** argv)
{
    init();

    SDL_Event event;
    while (game.state != QUIT) {

		event_handler();

        // Render background
		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
		SDL_Rect rect = {0, 0, width, height}; 
		SDL_RenderFillRect(renderer, &rect);
		SDL_Color textColor = {255, 255, 255};

        switch(game.state) {
            case ON:
                
				update_ball();
                render_score();

				// Render players / ball
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
		    	SDL_RenderFillRect(renderer, &player1);
    			SDL_RenderFillRect(renderer, &player2);
    			SDL_RenderFillRect(renderer, &ball);

				// if scored
		    	if(game.goal) {
	    			char player_point[20];
    				sprintf(player_point, "Player %d made a point", game.goal);
				    SDL_Surface *textSurface = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 40), player_point, textColor);
			    	SDL_Texture *textTexture3 = SDL_CreateTextureFromSurface(renderer, textSurface);
		    		SDL_Rect textRect = {(width - textSurface->w)/2, height/2 - 5*textSurface->h, textSurface->w, textSurface->h};
	    			SDL_RenderCopy(renderer, textTexture3, NULL, &textRect);
    				if(ballparam.vx < 0) ballparam.vx = -ispeed;
			    	else ballparam.vx = ispeed;
		    		ballparam.vy = -ballparam.vy;
	    			ballparam.bounced_tot = 0;
    			}
                break;
            case MENU:
                render_menu();
                break;
			case GAME_MENU:
				render_onlinemenu();
				break;
            case OPTIONS:
                render_options();
                break;
            case END:
                render_winner();
	    		game.score1 = 0;
		    	game.score2 = 0;
    			ballparam.bounced = 0;
	    		ballparam.bounced_tot = 0;
                break;    
			default:
				break;
        }

    	SDL_RenderPresent(renderer);
		// add delay
		if(game.goal) {
			SDL_Delay(990);
			game.goal = 0;
			    ball.x = ballparam.ix;
			    ball.y = ballparam.iy;
		} 
		if(game.state == END) {
			SDL_Delay(990);
            game.state = MENU;
		}
 		SDL_Delay(10);
    }

    // Free resources and close SDL
    SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
 
    return 0;
}

void event_handler() {
	SDL_Event event;
	//event management
	if (SDL_PollEvent(&event)) {
       	switch (event.type) {
           	case SDL_QUIT:
               	break;
			case SDL_WINDOWEVENT:
				if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
					width = event.window.data1;
					height = event.window.data2;
					hrect = height/4;
					player1.y = height/2 - hrect/2;
					player2.y = height/2 - hrect/2;
					player1.x = wrect*8;
					player2.x = width - wrect*8;
					ballparam.ix = width/2;
					ballparam.iy = height/2;
					ball.x = ballparam.ix;
					ball.y = ballparam.iy;
				}
				break;		
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
					case SDLK_1:
						if (game.state == OPTIONS) {
							if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) {
								SDL_SetWindowFullscreen(window, 0);
							} else {
								SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
							}
						} else if(game.state == GAME_MENU) {
							game.state = ON;
						}
						break;
					case SDLK_2:
						if(game.state == GAME_MENU) {
							game.state = ONLINE;
						}
					// Echap
					case SDLK_ESCAPE:
						if(game.state == MENU) {
							game.state = QUIT;
						} else if(game.state == ON || game.state == ONLINE) {
							Mix_PlayChannel(-1, sound_endgame, 0);
							game.state = END;
							game.goal = 0;
						} else if(game.state == OPTIONS) {
							Mix_PlayChannel(-1, sound_menu, 0);
							game.state = MENU;
						}
						break;
					// Espace
					case SDLK_SPACE:
						if(game.state == MENU) {
							Mix_PlayChannel(-1, sound_menu, 0);
							game.state = OPTIONS;
							}	
						break;
					// Return
					case SDLK_RETURN:
						if (game.state == MENU) {
							Mix_PlayChannel(-1, sound_menu, 0);
							game.state = GAME_MENU;
						}
						break;
					// Player 1
					case SDLK_z:
						if(player1.y > -hrect/2) player1.y -= pspeed;
						break;
					case SDLK_s:
						if(player1.y < height-hrect/2) player1.y += pspeed;
						break;
					// Player 2
					case SDLK_UP:
						if(player2.y > -hrect/2) player2.y -= pspeed;
						break;
					case SDLK_DOWN:
						if(player2.y < height-hrect/2) player2.y += pspeed;
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}
void update_ball() {
	//update ball position
                if (ball.y < 0) {
				    Mix_PlayChannel(-1, sound_bounce_wall, 0);
				    ball.y = 0;
				    ballparam.vy = -ballparam.vy;
			    }
    			if (ball.y+ball.w > height) {
    				Mix_PlayChannel(-1, sound_bounce_wall, 0);
    				ball.y = height-ball.w;
    				ballparam.vy = -ballparam.vy;
    			}
    			if (ball.x < 0) {
    				Mix_PlayChannel(-1, sound_point, 0);
    				ballparam.vx = -ballparam.vx;
    				game.score2++;  
	    			game.goal = 2;
		    	}
    			if (ball.x > width-ball.w) {
	    			Mix_PlayChannel(-1, sound_point, 0);
		    		ballparam.vx = -ballparam.vx;
			    	game.score1++;
				    game.goal = 1;
    			}
	    		if(ball.x >= player1.x && ball.x <= player1.x+ball.w && ball.y >= player1.y && ball.y <= player1.y+hrect) {
		    		Mix_PlayChannel(-1, sound_bounce, 0);
			    	ballparam.vx = -ballparam.vx;
    				ball.x = player1.x+ball.w;
	    			ballparam.bounced++;
		    		ballparam.bounced_tot++;
			    	if(ballparam.bounced_tot == 1) {
	    				ballparam.vx = ispeed;
    					if(ballparam.vy < 0) ballparam.vy = -(ispeed - ispeed/2);
		    			else ballparam.vy = ispeed - ispeed/2;
			    	}
    				if(ballparam.bounced >= 4 && ballparam.vx <= 18 && ballparam.vx >= -18) {
	    				ballparam.bounced = 0;
		    			if(ballparam.vx < 0) ballparam.vx--;
			    		else ballparam.vx++;
				    }
			    }
			    if(ball.x+ball.w >= player2.x && ball.x+ball.w <= player2.x+ball.w && ball.y >= player2.y && ball.y <= player2.y+hrect) {
    				Mix_PlayChannel(-1, sound_bounce, 0);
	    			ballparam.vx = -ballparam.vx;
		    		ball.x = player2.x-ball.w;
    				ballparam.bounced++;
	    			ballparam.bounced_tot++;
    				if(ballparam.bounced_tot == 1) {
	    				ballparam.vx = ispeed;
			    		if(ballparam.vy < 0) ballparam.vy = -(ispeed - ispeed/2);
		    			else ballparam.vy = ispeed - ispeed/2;
				    }
				    if(ballparam.bounced >= 4 && ballparam.vx <= 18 && ballparam.vx >= -18) {
					    ballparam.bounced = 0;
    					if(ballparam.vx < 0) ballparam.vx--;
	    				else ballparam.vx++;
		    		}
			    }
			    ball.x += ballparam.vx;
			    ball.y += ballparam.vy;
}

void init() {

    game.state = MENU;

	//Initliaze SDL
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		game.state = QUIT;
	}
	TTF_Init();

	//get screen size
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	width = dm.w;
	height = dm.h;

    //Create a window
	window = SDL_CreateWindow("SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);

	//allow resize window
	SDL_SetWindowResizable(window, SDL_TRUE);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	//Sound initialization 
	if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048) < 0) {
		printf("Mix_OpenAudio: %s\n", Mix_GetError());
		game.state = QUIT;
	}
	Mix_AllocateChannels(2);

	//load wav
	sound_bounce = Mix_LoadWAV_RW(SDL_RWFromFile("SoundEffects/bounce.wav","rb"), 1);
	if(!sound_bounce) {
		printf("Mix_LoadWAV Error : %s\n", Mix_GetError());
		game.state = QUIT;
	}
	sound_menu = Mix_LoadWAV_RW(SDL_RWFromFile("SoundEffects/menu.wav","rb"), 1);
	if(!sound_menu) {
		printf("Mix_LoadWAV Error : %s\n", Mix_GetError());
		game.state = QUIT;
	}
	sound_bounce_wall = Mix_LoadWAV_RW(SDL_RWFromFile("SoundEffects/bounce_wall.wav","rb"), 1);
	if(!sound_bounce_wall) {
		printf("Mix_LoadWAV Error : %s\n", Mix_GetError());
		game.state = QUIT;
	}
	sound_point = Mix_LoadWAV_RW(SDL_RWFromFile("SoundEffects/point.wav","rb"), 1);
	if(!sound_point) {
		printf("Mix_LoadWAV Error : %s\n", Mix_GetError());
		game.state = QUIT;
	}
	sound_endgame = Mix_LoadWAV_RW(SDL_RWFromFile("SoundEffects/endgame.wav","rb"), 1);
	if(!sound_endgame) {
		printf("Mix_LoadWAV Error : %s\n", Mix_GetError());
		game.state = QUIT;
	}

    init_players();
    init_ball();
}
void init_ball() {
    //initliaze ball parameters
    ballparam.ix = width/2;
    ballparam.iy = height/2;
    ballparam.vx = ispeed - ispeed/4;
    ballparam.vy = 0;
    ballparam.bounced = 0;
    ballparam.bounced_tot = 0;

    ball.w = 30;
    ball.h = 30;
    ball.x = ballparam.ix;
    ball.y = ballparam.iy;
}
void init_players() {
    //Init players rectangles
    hrect = height/4;
    wrect = 30;
	pspeed = 70;
    player1.w = wrect;
    player1.h = hrect;
    player2.w = wrect;
    player2.h = hrect;
	player1.y = height/2 - hrect/2;
    player2.y = height/2 - hrect/2;
	player1.x = wrect*8;
    player2.x = width - wrect*8;
}
void init_game() {
    //Init game parameters
    game.score1 = 0;
    game.score2 = 0;
    game.goal = 0;
    init_ball();
}

void render_menu() {
	SDL_Color textColor = {255, 255, 255};
	// render Welcome text
	SDL_Surface *textSurface = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 64), "Welcome to Pong", textColor);
	SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_Rect textRect = {(width - textSurface->w) / 2, height / 2 - 5 * textSurface->h, textSurface->w, textSurface->h};
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

	// render Start game text
	SDL_Surface *textSurface2 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 24), "Press enter to start", textColor);
	SDL_Texture *textTexture2 = SDL_CreateTextureFromSurface(renderer, textSurface2);
	SDL_Rect textRect2 = {(width - textSurface2->w) / 2, height / 2 - textSurface2->h, textSurface2->w, textSurface2->h};
	SDL_RenderCopy(renderer, textTexture2, NULL, &textRect2);

	// render options text
	SDL_Surface *textSurface3 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 24), "Press ESP for options", textColor);
	SDL_Texture *textTexture3 = SDL_CreateTextureFromSurface(renderer, textSurface3);
	SDL_Rect textRect3 = {(width - textSurface3->w) / 2, height / 2 + textSurface3->h, textSurface3->w, textSurface3->h};
	SDL_RenderCopy(renderer, textTexture3, NULL, &textRect3);

	// render quit text
	SDL_Surface *textSurface4 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 24), "Press ESC to quit game", textColor);
	SDL_Texture *textTexture4 = SDL_CreateTextureFromSurface(renderer, textSurface4);
	SDL_Rect textRect4 = {(width - textSurface4->w) / 2, height / 2 + 3 * textSurface4->h, textSurface4->w, textSurface4->h};
	SDL_RenderCopy(renderer, textTexture4, NULL, &textRect4);
}
void render_options() {
	SDL_Color textColor = {255, 255, 255};
	// render text to change ball speed
	SDL_Surface *textSurface5 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 24), "Press 1 for fullscreen", textColor);
	SDL_Texture *textTexture5 = SDL_CreateTextureFromSurface(renderer, textSurface5);
	SDL_Rect textRect5 = {(width - textSurface5->w) / 2, height / 2 - textSurface5->h, textSurface5->w, textSurface5->h};
	SDL_RenderCopy(renderer, textTexture5, NULL, &textRect5);

	// render back to main menu
	SDL_Surface *textSurface6 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 24), "Press ESC to quit options", textColor);
	SDL_Texture *textTexture6 = SDL_CreateTextureFromSurface(renderer, textSurface6);
	SDL_Rect textRect4 = {(width - textSurface6->w) / 2, height / 2 + 3 * textSurface6->h, textSurface6->w, textSurface6->h};
	SDL_RenderCopy(renderer, textTexture6, NULL, &textRect4);
}
void render_score() {
	SDL_Color textColor = {255, 255, 255};
	// save int into char* variable
	char score1_str[10];
	sprintf(score1_str, "%d", game.score1);
	char score2_str[10];
	sprintf(score2_str, "%d", game.score2);

	// draw score
	SDL_Surface *textSurface = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 40), score1_str, textColor);
	SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_Rect textRect = {width / 2 - width / 15, height / 20 + textSurface->h, textSurface->w, textSurface->h};
	SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
	SDL_Surface *textSurface2 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 40), score2_str, textColor);
	SDL_Texture *textTexture2 = SDL_CreateTextureFromSurface(renderer, textSurface2);
	SDL_Rect textRect2 = {width / 2 + width / 15, height / 20 + textSurface2->h, textSurface2->w, textSurface2->h};
	SDL_RenderCopy(renderer, textTexture2, NULL, &textRect2);
}
void render_winner() {
	SDL_Color textColor = {255, 255, 255};
	if (game.score1 > game.score2) {
		SDL_Surface *textSurface7 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 64), "Player 1 WIN !!!", textColor);
		SDL_Texture *textTexture7 = SDL_CreateTextureFromSurface(renderer, textSurface7);
		SDL_Rect textRect = {(width - textSurface7->w) / 2, height / 2 - 5 * textSurface7->h, textSurface7->w, textSurface7->h};
		SDL_RenderCopy(renderer, textTexture7, NULL, &textRect);
	} else if (game.score1 == game.score2) {
		SDL_Surface *textSurface8 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 64), "Spare", textColor);
		SDL_Texture *textTexture8 = SDL_CreateTextureFromSurface(renderer, textSurface8);
		SDL_Rect textRect = {(width - textSurface8->w) / 2, height / 2 - 5 * textSurface8->h, textSurface8->w, textSurface8->h};
		SDL_RenderCopy(renderer, textTexture8, NULL, &textRect);
	} else {
		SDL_Surface *textSurface9 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 64), "Player 2 WIN !!!", textColor);
		SDL_Texture *textTexture9 = SDL_CreateTextureFromSurface(renderer, textSurface9);
		SDL_Rect textRect = {(width - textSurface9->w) / 2, height / 2 - 5 * textSurface9->h, textSurface9->w, textSurface9->h};
		SDL_RenderCopy(renderer, textTexture9, NULL, &textRect);
	}
}
void render_onlinemenu() {
	SDL_Color textColor = {255, 255, 255};
	// render text to change ball speed
	SDL_Surface *textSurface5 = TTF_RenderText_Solid(TTF_OpenFont("fonts/bit5x3.ttf", 24), "Press 1 for fullscreen", textColor);
	SDL_Texture *textTexture5 = SDL_CreateTextureFromSurface(renderer, textSurface5);
	SDL_Rect textRect5 = {(width - textSurface5->w) / 2, height / 2 - textSurface5->h, textSurface5->w, textSurface5->h};
	SDL_RenderCopy(renderer, textTexture5, NULL, &textRect5);
}