#include "perso.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 767
#define FULLSCREEN_WIDTH 1920	
#define FULLSCREEN_HEIGHT 1080

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    printf("SDL initialized\n");

    if (TTF_Init() < 0) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    printf("SDL_ttf initialized\n");

    TTF_Font *font = TTF_OpenFont("arial.ttf", 20);
    if (font == NULL) {
        printf("Failed to load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int is_fullscreen = 0;
    int current_width = SCREEN_WIDTH;
    int current_height = SCREEN_HEIGHT;
    SDL_Surface *screen = SDL_SetVideoMode(current_width, current_height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    if (!screen) {
        printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    printf("Screen created: %dx%d, format=%d bpp\n", current_width, current_height, screen->format->BitsPerPixel);

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf("IMG_Init failed: %s\n", IMG_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    printf("SDL_image initialized\n");

    perso player1, player2;
    init_perso(&player1);
    init_perso(&player2);
    player2.pos.x = 662;
    int player2_visible = 0;

    SDL_Surface *optimized1[6];
    SDL_Surface *optimized2[6];
    for (int i = 0; i < 6; i++) {
        optimized1[i] = SDL_DisplayFormat(player1.images[i]);
        optimized2[i] = SDL_DisplayFormat(player2.images[i]);
        if (optimized1[i] && optimized2[i]) {
            SDL_FreeSurface(player1.images[i]);
            SDL_FreeSurface(player2.images[i]);
            player1.images[i] = optimized1[i];
            player2.images[i] = optimized2[i];
        } else {
            printf("Warning: Failed to optimize sprite surfaces for state %d\n", i);
        }
    }
    printf("Sprite surfaces optimized\n");
    printf("Players initialized: p1.x=%d, p1.y=%d, p2.x=%d, p2.y=%d\n", 
           player1.pos.x, player1.pos.y, player2.pos.x, player2.pos.y);

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        running = 0;
                        break;
                    case SDLK_m:
                        is_fullscreen = !is_fullscreen;
                        if (is_fullscreen) {
                            current_width = FULLSCREEN_WIDTH;
                            current_height = FULLSCREEN_HEIGHT;
                            screen = SDL_SetVideoMode(current_width, current_height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
                        } else {
                            current_width = SCREEN_WIDTH;
                            current_height = SCREEN_HEIGHT;
                            screen = SDL_SetVideoMode(current_width, current_height, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
                        }
                        if (!screen) {
                            printf("SDL_SetVideoMode failed: %s\n", SDL_GetError());
                            running = 0;
                        }
                        break;
                    case SDLK_p:
                        player2_visible = !player2_visible;
                        if (!player2_visible) {
                            player2.pos.x = 662;
                            player2.state = IDLE;
                            player2.currentFrame = 0;
                        }
                        break;
                    case SDLK_LSHIFT:
                        if (player2_visible) {
                            attack_perso(&player2);
                        }
                        break;
                    case SDLK_j:
                        trigger_hit(&player1);
                        break;
                    case SDLK_k:
                        if (player2_visible) {
                            trigger_hit(&player2);
                        }
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    attack_perso(&player1);
                }
            }
        }

        deplacer_perso(&player1, current_width);
        jump_perso(&player1);
        if (player2_visible) {
            deplacer_perso1(&player2, current_width);
            jump_perso1(&player2);
            animer_perso(&player2);
        }
        animer_perso(&player1);

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        printf("Screen cleared to black\n");

        SDL_Rect render_pos1 = {player1.pos.x, player1.pos.y, 0, 0};
        SDL_Rect render_pos2 = {player2.pos.x, player2.pos.y, 0, 0};
        printf("Render: p1.x=%d, p1.y=%d, p2.x=%d, p2.y=%d\n", 
               render_pos1.x, render_pos1.y, render_pos2.x, render_pos2.y);

        afficher_perso(&player1, screen, &render_pos1);
        if (player2_visible) {
            afficher_perso(&player2, screen, &render_pos2);
        }

        afficher_score_vie(&player1, screen, 1, font);
        if (player2_visible) {
            afficher_score_vie(&player2, screen, 2, font);
        }

        SDL_Flip(screen);
        printf("Screen updated\n");

        SDL_Delay(16);
    }

    free_perso(&player1);
    free_perso(&player2);
    TTF_CloseFont(font);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    printf("Cleanup complete\n");
    return 0;
}
