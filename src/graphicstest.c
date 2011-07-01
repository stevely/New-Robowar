/*
 * graphicstest.c
 *
 * By Steven Smith
 */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_main.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "battlehandler.h"

static SDL_Surface *screen;
static SDL_Surface *robo;
static SDL_Surface *bullet;
static TTF_Font *font;

static void load_texture( const char *fname, SDL_Surface **sp ) {
    SDL_Surface *image;
    image = IMG_Load(fname);
    if( image ) { /* Create optimized sprite */
        *sp = SDL_DisplayFormatAlpha(image);
        if( *sp ) {
            SDL_FreeSurface(image);
        }
        else {
            *sp = image;
        }
    }
    else {
        fprintf(stdout, "load_texture\n");
    }
}

static SDL_Surface * load_hp_text( RW_Active_Robot *bot ) {
    SDL_Color color = {0, 0, 0, 0};
    char text[10];
    sprintf(text, "%d", bot->damage);
    return TTF_RenderText_Blended(font, text, color);
}

static void draw_text( SDL_Surface *text, int x, int y) {
    SDL_Rect dest;
    if( text ) {
        dest.x = x;
        dest.y = y;
        dest.w = text->w;
        dest.h = text->h;
        SDL_BlitSurface(text, NULL, screen, &dest);
        SDL_FreeSurface(text);
    }
}

static void draw_sprite( SDL_Surface *tex, int x, int y) {
    SDL_Rect dest;
    if( tex ) {
        dest.x = x;
        dest.y = y;
        dest.w = tex->w;
        dest.h = tex->h;
        SDL_BlitSurface(tex, NULL, screen, &dest);
    }
}

static void display( RW_Battle *b ) {
    SDL_Surface *text;
    RW_Active_Robot *bot;
    RW_Shot *shot;
    RW_Robot_Iter ri;
    RW_Shot_Iter si;
    SDL_FillRect(screen, NULL, 0xFFFFFFFF);
    RW_Reset_Shot_Iter(b, &si);
    while( (shot = RW_Shot_Next(&si)) ) {
        draw_sprite(bullet, shot->x - 1, shot->y - 1);
    }
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next(&ri)) ) {
        draw_sprite(robo, bot->regs[reg_x] - 9, bot->regs[reg_y] - 9);
    }
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next(&ri)) ) {
        text = load_hp_text(bot);
        draw_text(text, 3, 3 + (bot->id * 14));
    }
    SDL_Flip(screen);
}

int main( int argc, char **argv ) {
    int fps, keyflag;
    RW_Battle *b;
    RW_Robot *bot1, *bot2;
    SDL_Event event;
    if( argc != 3 ) {
        fprintf(stdout, "Usage: %s robot robot\n", argv[0]);
        return 0;
    }
    bot1 = RW_Read_Robot(argv[1]);
    bot2 = RW_Read_Robot(argv[2]);
    RW_Reset_Scores();
    b = RW_New_Battle();
    RW_Setup_Duel(b, bot1, bot2);
    if( SDL_Init(SDL_INIT_TIMER) ) {
        return -1;
    }
    if( SDL_InitSubSystem(SDL_INIT_VIDEO) ) {
        return -1;
    }
    if( TTF_Init() ) {
        return -1;
    }
    screen = SDL_SetVideoMode(300, 300, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_WM_SetCaption("New RoboWar", "New RoboWar");
    load_texture("bullet.png", &bullet);
    load_texture("robot.png", &robo);
    font = TTF_OpenFont("/System/Library/Fonts/HelveticaLight.ttf", 12);
    fps = 32;
    keyflag = 0;
    while( RW_Run_Chronon(b) ) {
        SDL_Delay(1000 / fps); /* 30 FPS */
        while( SDL_PollEvent(&event) ) {
            if( event.type == SDL_KEYDOWN && !keyflag ) {
                keyflag = !keyflag;
                switch(event.key.keysym.sym) {
                    case SDLK_q:
                        SDL_Quit();
                        return 0;
                    case SDLK_LEFT:
                        fps /= 2;
                        break;
                    case SDLK_RIGHT:
                        fps *= 2;
                        break;
                    default:
                        break;
                }
            }
            else if( event.type == SDL_KEYUP && keyflag ) {
                keyflag = !keyflag;
            }
        }
        display(b);
    }
    fprintf(stdout, "Result of battle after %d chronons:\n", b->chronon);
    fprintf(stdout, "%d: %d points\n", b->bots[0].id, b->score[0]);
    fprintf(stdout, "%d: %d points\n", b->bots[1].id, b->score[1]);
    SDL_Quit();
    return 0;
}
