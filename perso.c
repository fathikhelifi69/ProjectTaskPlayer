#include "perso.h"
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1024
#define ACCELERATION 0.1
#define ACCEL_DELAY 500

void init_perso(perso *p) {
    // Load each state's spritesheet
    const char* filenames[6] = {
        "Idle.png",   // IDLE
        "Run.png",    // RUN
        "Attack.png", // ATTACK
        "Jump.png",   // JUMP
        "Hurt.png",   // HURT
        "Dead.png"    // DEAD
    };

    for (int i = 0; i < 6; i++) {
        p->images[i] = IMG_Load(filenames[i]);
        if (!p->images[i]) {
            printf("Error loading %s: %s\n", filenames[i], IMG_GetError());
            // Free any previously loaded surfaces before exiting
            for (int j = 0; j < i; j++) {
                SDL_FreeSurface(p->images[j]);
            }
            exit(1);
        }

        // Set transparency (white background: RGB 255,255,255)
        Uint32 colorkey = SDL_MapRGB(p->images[i]->format, 255, 255, 255);
        if (SDL_SetColorKey(p->images[i], SDL_SRCCOLORKEY, colorkey) != 0) {
            printf("Error setting colorkey for %s: %s\n", filenames[i], SDL_GetError());
        }
        if (p->images[i]->format->Amask) {
            SDL_SetAlpha(p->images[i], SDL_SRCALPHA, 255);
        }
    }

    p->pos.x = 50;
    p->pos.y = GROUND_LEVEL;
    p->pos.w = 128;
    p->pos.h = 128;

    p->frameRect.x = 0;
    p->frameRect.y = 0;
    p->frameRect.w = 128;
    p->frameRect.h = 128;

    p->state = IDLE;
    p->direction = 0;
    p->currentFrame = 0;
    p->frameCounts[IDLE] = 13;
    p->frameCounts[RUN] = 10;
    p->frameCounts[ATTACK] = 6;
    p->frameCounts[JUMP] = 10;
    p->frameCounts[HURT] = 3;
    p->frameCounts[DEAD] = 5;

    p->animSpeeds[IDLE] = 80;
    p->animSpeeds[RUN] = 60;
    p->animSpeeds[ATTACK] = 50;
    p->animSpeeds[JUMP] = 70;
    p->animSpeeds[HURT] = 100;
    p->animSpeeds[DEAD] = 120;

    p->lastUpdate = SDL_GetTicks();

    p->vie = 100;
    p->score = 0;
    p->last_hit_time = 0;
    p->velocity_y = 0;
    p->is_jumping = 0;
    p->played_dead = 0;
    p->is_dead = 0;
    printf("Perso init: x=%d, y=%d, state=%d\n", p->pos.x, p->pos.y, p->state);
}

void animer_perso(perso *p) {
    if (p->is_dead && p->played_dead) return;

    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - p->lastUpdate < p->animSpeeds[p->state]) return;

    p->currentFrame = (p->currentFrame + 1) % p->frameCounts[p->state];
    p->frameRect.x = p->currentFrame * 128;
    p->frameRect.y = 0; // Each surface contains only one row

    if (p->state == HURT && p->currentFrame == p->frameCounts[HURT] - 1) {
        p->state = p->vie > 0 ? IDLE : DEAD;
        p->currentFrame = 0;
    }
    if (p->state == ATTACK && p->currentFrame == p->frameCounts[ATTACK] - 1) {
        p->state = IDLE;
        p->currentFrame = 0;
    }
    if (p->state == DEAD && p->currentFrame == p->frameCounts[DEAD] - 1) {
        p->played_dead = 1;
        p->is_dead = 1;
    }

    p->lastUpdate = currentTime;
    printf("Perso anim: state=%d, frame=%d, x=%d, y=%d, direction=%d, played_dead=%d\n", 
           p->state, p->currentFrame, p->frameRect.x, p->frameRect.y, p->direction, p->played_dead);
}

void deplacer_perso(perso *p, int screen_width) {
    if (p->is_dead || p->state == DEAD || p->state == ATTACK || p->state == HURT) return;

    const Uint8 *keys = SDL_GetKeyState(NULL);
    static Uint32 last_move_time = 0;
    static int moving = 0;
    static float speed = 4.0;
    float dt = 16.0 / 1000.0;
    int moved = 0;

    if (keys[SDLK_LEFT]) {
        if (!moving) {
            last_move_time = SDL_GetTicks();
            moving = 1;
            speed = 4.0;
        }
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_move_time > ACCEL_DELAY) {
            speed += ACCELERATION * (current_time - last_move_time - ACCEL_DELAY) * dt;
            if (speed > 8.0) speed = 8.0;
        }
        p->pos.x -= (int)speed;
        p->direction = 1;
        if (!p->is_jumping) p->state = RUN;
        moved = 1;
        printf("Perso moving left: x=%d, direction=%d, state=%d, speed=%f\n", p->pos.x, p->direction, p->state, speed);
    }
    else if (keys[SDLK_RIGHT]) {
        if (!moving) {
            last_move_time = SDL_GetTicks();
            moving = 1;
            speed = 4.0;
        }
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_move_time > ACCEL_DELAY) {
            speed += ACCELERATION * (current_time - last_move_time - ACCEL_DELAY) * dt;
            if (speed > 8.0) speed = 8.0;
        }
        p->pos.x += (int)speed;
        p->direction = 0;
        if (!p->is_jumping) p->state = RUN;
        moved = 1;
        printf("Perso moving right: x=%d, direction=%d, state=%d, speed=%f\n", p->pos.x, p->direction, p->state, speed);
    }
    else {
        moving = 0;
        speed = 4.0;
    }

    if (p->pos.x < 0) p->pos.x = 0;
    if (p->pos.x + p->pos.w > screen_width) p->pos.x = screen_width - p->pos.w;

    if (!moved && !p->is_jumping && p->state != ATTACK) {
        p->state = IDLE;
    }

    printf("Perso move: x=%d, state=%d, direction=%d\n", p->pos.x, p->state, p->direction);
}

void jump_perso(perso *p) {
    if (p->is_dead || p->state == DEAD || p->state == ATTACK || p->state == HURT) return;

    const Uint8 *keys = SDL_GetKeyState(NULL);
    if (keys[SDLK_UP] && !p->is_jumping) {
        p->velocity_y = -15;
        p->is_jumping = 1;
        p->state = JUMP;
        p->currentFrame = 0;
        printf("Perso jump: y=%d, velocity_y=%f\n", p->pos.y, p->velocity_y);
    }

    if (p->is_jumping) {
        p->pos.y += p->velocity_y;
        p->velocity_y += 0.5;

        if (p->pos.y >= GROUND_LEVEL) {
            p->pos.y = GROUND_LEVEL;
            p->velocity_y = 0;
            p->is_jumping = 0;
            p->state = IDLE;
            p->currentFrame = 0;
            printf("Perso land: y=%d\n", p->pos.y);
        }
    }
}

void deplacer_perso1(perso *p, int screen_width) {
    if (p->is_dead || p->state == DEAD || p->state == ATTACK || p->state == HURT) return;

    const Uint8 *keys = SDL_GetKeyState(NULL);
    static Uint32 last_move_time = 0;
    static int moving = 0;
    static float speed = 4.0;
    float dt = 16.0 / 1000.0;
    int moved = 0;

    if (keys[SDLK_q]) {
        if (!moving) {
            last_move_time = SDL_GetTicks();
            moving = 1;
            speed = 4.0;
        }
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_move_time > ACCEL_DELAY) {
            speed += ACCELERATION * (current_time - last_move_time - ACCEL_DELAY) * dt;
            if (speed > 8.0) speed = 8.0;
        }
        p->pos.x -= (int)speed;
        p->direction = 1;
        if (!p->is_jumping) p->state = RUN;
        moved = 1;
        printf("Perso1 moving left: x=%d, direction=%d, state=%d, speed=%f\n", p->pos.x, p->direction, p->state, speed);
    }
    else if (keys[SDLK_d]) {
        if (!moving) {
            last_move_time = SDL_GetTicks();
            moving = 1;
            speed = 4.0;
        }
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_move_time > ACCEL_DELAY) {
            speed += ACCELERATION * (current_time - last_move_time - ACCEL_DELAY) * dt;
            if (speed > 8.0) speed = 8.0;
        }
        p->pos.x += (int)speed;
        p->direction = 0;
        if (!p->is_jumping) p->state = RUN;
        moved = 1;
        printf("Perso1 moving right: x=%d, direction=%d, state=%d, speed=%f\n", p->pos.x, p->direction, p->state, speed);
    }
    else {
        moving = 0;
        speed = 4.0;
    }

    if (p->pos.x < 0) p->pos.x = 0;
    if (p->pos.x + p->pos.w > screen_width) p->pos.x = screen_width - p->pos.w;

    if (!moved && !p->is_jumping && p->state != ATTACK) {
        p->state = IDLE;
    }

    printf("Perso1 move: x=%d, state=%d, direction=%d\n", p->pos.x, p->state, p->direction);
}

void jump_perso1(perso *p) {
    if (p->is_dead || p->state == DEAD || p->state == ATTACK || p->state == HURT) return;

    const Uint8 *keys = SDL_GetKeyState(NULL);
    if (keys[SDLK_z] && !p->is_jumping) {
        p->velocity_y = -15;
        p->is_jumping = 1;
        p->state = JUMP;
        p->currentFrame = 0;
        printf("Perso1 jump: y=%d, velocity_y=%f\n", p->pos.y, p->velocity_y);
    }

    if (p->is_jumping) {
        p->pos.y += p->velocity_y;
        p->velocity_y += 0.5;

        if (p->pos.y >= GROUND_LEVEL) {
            p->pos.y = GROUND_LEVEL;
            p->velocity_y = 0;
            p->is_jumping = 0;
            p->state = IDLE;
            p->currentFrame = 0;
            printf("Perso1 land: y=%d\n", p->pos.y);
        }
    }
}

void trigger_hit(perso *p) {
    if (p->is_dead || p->state == DEAD || p->state == HURT) return;
    Uint32 current_time = SDL_GetTicks();
    if (current_time - p->last_hit_time < HIT_COOLDOWN) return;

    p->state = HURT;
    p->currentFrame = 0;
    p->lastUpdate = current_time;
    p->vie -= 20;
    p->last_hit_time = current_time;
    printf("Perso hit: vie=%d\n", p->vie);
    if (p->vie <= 0) {
        p->vie = 0;
        p->state = DEAD;
        p->currentFrame = 0;
        p->is_dead = 1;
        printf("Perso died\n");
    }
}

void attack_perso(perso *p) {
    if (p->is_dead || p->state == DEAD || p->state == ATTACK || p->state == HURT) return;

    p->state = ATTACK;
    p->currentFrame = 0;
    p->lastUpdate = SDL_GetTicks();
    printf("Perso attack: state=%d\n", p->state);
}

void afficher_perso(perso *p, SDL_Surface *screen, SDL_Rect *render_pos) {
    SDL_Surface *current_image = p->images[p->state];
    if (!current_image || !screen) {
        printf("Perso render error: invalid surface (image=%p, screen=%p)\n", current_image, screen);
        return;
    }

    SDL_Rect src_rect = p->frameRect;
    if (p->direction == 1) {
        SDL_Surface *flipped = SDL_CreateRGBSurface(current_image->flags, src_rect.w, src_rect.h, 
                                                   current_image->format->BitsPerPixel, 
                                                   current_image->format->Rmask, current_image->format->Gmask, 
                                                   current_image->format->Bmask, current_image->format->Amask);
        if (!flipped) {
            printf("Perso render error: failed to create flipped surface\n");
            return;
        }

        Uint32 colorkey = SDL_MapRGB(flipped->format, 255, 255, 255);
        SDL_SetColorKey(flipped, SDL_SRCCOLORKEY, colorkey);
        if (current_image->format->Amask) {
            SDL_SetAlpha(flipped, SDL_SRCALPHA, 255);
        }

        SDL_LockSurface(current_image);
        SDL_LockSurface(flipped);
        for (int x = 0; x < src_rect.w; x++) {
            for (int y = 0; y < src_rect.h; y++) {
                Uint32 pixel = get_pixel(current_image, src_rect.x + x, src_rect.y + y);
                put_pixel(flipped, src_rect.w - 1 - x, y, pixel);
            }
        }
        SDL_UnlockSurface(current_image);
        SDL_UnlockSurface(flipped);

        int result = SDL_BlitSurface(flipped, NULL, screen, render_pos);
        SDL_FreeSurface(flipped);
        if (result != 0) {
            printf("Perso render error: SDL_BlitSurface failed for flipped: %s\n", SDL_GetError());
        } else {
            printf("Perso render (flipped): x=%d, y=%d, state=%d, frame_x=%d, frame_y=%d\n", 
                   render_pos->x, render_pos->y, p->state, src_rect.x, src_rect.y);
        }
    } else {
        int result = SDL_BlitSurface(current_image, &src_rect, screen, render_pos);
        if (result != 0) {
            printf("Perso render error: SDL_BlitSurface failed: %s\n", SDL_GetError());
        } else {
            printf("Perso render: x=%d, y=%d, state=%d, frame_x=%d, frame_y=%d\n", 
                   render_pos->x, render_pos->y, p->state, src_rect.x, src_rect.y);
        }
    }
}

void afficher_score_vie(perso *p, SDL_Surface *screen, int player_num, TTF_Font *font) {
    if (p->is_dead && p->played_dead) return;

    SDL_Color text_color = {255, 255, 255};
    char player_label[10];
    sprintf(player_label, "Player %d", player_num);
    SDL_Surface *player_text = TTF_RenderText_Solid(font, player_label, text_color);
    if (player_text == NULL) {
        printf("Failed to render player label: %s\n", TTF_GetError());
        return;
    }

    char score_label[20];
    sprintf(score_label, "Score: %d", p->score);
    SDL_Surface *score_text = TTF_RenderText_Solid(font, score_label, text_color);
    if (score_text == NULL) {
        printf("Failed to render score label: %s\n", TTF_GetError());
        SDL_FreeSurface(player_text);
        return;
    }

    int health_bar_width = 100;
    SDL_Rect health_pos;
    SDL_Rect text_pos;
    SDL_Rect score_pos;
    if (player_num == 1) {
        health_pos.x = 10;
        text_pos.x = 10;
        score_pos.x = 10;
    } else {
        health_pos.x = screen->w - health_bar_width - 10;
        text_pos.x = screen->w - health_bar_width - 10;
        score_pos.x = screen->w - health_bar_width - 10;
    }
    health_pos.y = 30;
    health_pos.w = health_bar_width;
    health_pos.h = 10;
    text_pos.y = 5;
    score_pos.y = 45;

    SDL_Surface *health_bar_bg = SDL_CreateRGBSurface(0, health_bar_width, 10, 32, 0, 0, 0, 0);
    if (health_bar_bg == NULL) {
        printf("Failed to create health bar background surface: %s\n", SDL_GetError());
        SDL_FreeSurface(player_text);
        SDL_FreeSurface(score_text);
        return;
    }
    SDL_FillRect(health_bar_bg, NULL, SDL_MapRGB(health_bar_bg->format, 255, 0, 0));

    int health_width = p->vie > 0 ? p->vie : 0;
    SDL_Surface *health_bar = SDL_CreateRGBSurface(0, health_width, 10, 32, 0, 0, 0, 0);
    if (health_bar == NULL) {
        printf("Failed to create health bar surface: %s\n", SDL_GetError());
        SDL_FreeSurface(health_bar_bg);
        SDL_FreeSurface(player_text);
        SDL_FreeSurface(score_text);
        return;
    }
    SDL_FillRect(health_bar, NULL, SDL_MapRGB(health_bar->format, 0, 255, 0));

    SDL_Surface *border = SDL_CreateRGBSurface(0, health_bar_width + 2, 12, 32, 0, 0, 0, 0);
    if (border == NULL) {
        printf("Failed to create border surface: %s\n", SDL_GetError());
        SDL_FreeSurface(health_bar);
        SDL_FreeSurface(health_bar_bg);
        SDL_FreeSurface(player_text);
        SDL_FreeSurface(score_text);
        return;
    }
    SDL_FillRect(border, NULL, SDL_MapRGB(border->format, 255, 255, 255));

    SDL_Rect inner = {1, 1, health_bar_width, 10};
    SDL_BlitSurface(health_bar_bg, NULL, border, &inner);

    inner.w = health_width;
    SDL_BlitSurface(health_bar, NULL, border, &inner);

    SDL_BlitSurface(player_text, NULL, screen, &text_pos);
    SDL_BlitSurface(border, NULL, screen, &health_pos);
    SDL_BlitSurface(score_text, NULL, screen, &score_pos);

    SDL_FreeSurface(health_bar);
    SDL_FreeSurface(health_bar_bg);
    SDL_FreeSurface(border);
    SDL_FreeSurface(player_text);
    SDL_FreeSurface(score_text);
}

Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        printf("get_pixel: out of bounds x=%d, y=%d\n", x, y);
        return 0;
    }

    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    switch (bpp) {
        case 1: return *p;
        case 2: return *(Uint16 *)p;
        case 4: return *(Uint32 *)p;
        default: return 0;
    }
}

void put_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) {
        printf("put_pixel: out of bounds x=%d, y=%d\n", x, y);
        return;
    }

    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    switch (bpp) {
        case 1: *p = pixel; break;
        case 2: *(Uint16 *)p = pixel; break;
        case 4: *(Uint32 *)p = pixel; break;
    }
}

void free_perso(perso *p) {
    for (int i = 0; i < 6; i++) {
        if (p->images[i]) {
            SDL_FreeSurface(p->images[i]);
            p->images[i] = NULL;
        }
    }
}
