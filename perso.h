#ifndef PERSO_H
#define PERSO_H

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#define GROUND_LEVEL 900 // Fits within 767-pixel world
#define HIT_COOLDOWN 1000

typedef enum {
    IDLE,
    RUN,
    ATTACK,
    JUMP,
    HURT,
    DEAD
} PersoState;

typedef struct {
    SDL_Surface* images[6]; // One surface per state
    SDL_Rect pos;
    SDL_Rect frameRect;
    PersoState state;
    int direction; // 0: right, 1: left
    int currentFrame;
    int frameCounts[6];
    int animSpeeds[6];
    Uint32 lastUpdate;
    int vie;
    int score;
    Uint32 last_hit_time;
    float velocity_y;
    int is_jumping;
    int played_dead;
    int is_dead;
} perso;

void init_perso(perso* p);
void animer_perso(perso* p);
void deplacer_perso(perso* p, int screen_width);
void jump_perso(perso* p);
void deplacer_perso1(perso* p, int screen_width);
void jump_perso1(perso* p);
void trigger_hit(perso* p);
void attack_perso(perso* p);
void afficher_perso(perso* p, SDL_Surface* screen, SDL_Rect* render_pos);
void afficher_score_vie(perso* p, SDL_Surface* screen, int player_num, TTF_Font* font);
Uint32 get_pixel(SDL_Surface *surface, int x, int y);
void put_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void free_perso(perso* p); // Add function to free surfaces

#endif
