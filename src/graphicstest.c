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
static SDL_Surface *robo[6];
static SDL_Surface *bullet;
static SDL_Surface *explosion36;
static SDL_Surface *explosion24;
static SDL_Surface *explosion12;
static SDL_Surface *stunner;
static SDL_Surface *hellbore;
static TTF_Font *font;
static int bot_count = 0;
static int fps = 32;
static int keyflag = 0;

static SDL_Surface * load_texture( const char *fname ) {
    SDL_Surface *image;
    SDL_Surface *result;
    image = IMG_Load(RW_Build_Path("img", fname));
    if( image ) { /* Create optimized sprite */
        result = SDL_DisplayFormatAlpha(image);
        if( result ) {
            SDL_FreeSurface(image);
        }
        else {
            result = image;
        }
        return result;
    }
    else {
        fprintf(stdout, "load_texture\n");
        return NULL;
    }
}

static TTF_Font * load_font( const char *fname, int size ) {
    return TTF_OpenFont(RW_Build_Path("fonts", fname), size);
}

static SDL_Surface * load_text( int val ) {
    SDL_Color color = {0, 0, 0, 0};
    char text[10];
    sprintf(text, "%d", val);
    return TTF_RenderText_Blended(font, text, color);
}

static SDL_Surface * load_name( char *name ) {
    SDL_Color color = {0, 0, 0, 0};
    return TTF_RenderText_Blended(font, name, color);
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

static void display_scores( RW_Battle *b ) {
    SDL_Surface *text;
    RW_Active_Robot *bot;
    RW_Robot_Iter ri;
    SDL_Rect r = {300, 0, 100, 300};
    SDL_FillRect(screen, &r, 0x88888888);
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next(&ri)) ) {
        text = load_name(RW_Get_Robot_Name(bot));
        draw_text(text, 303, 3 + (bot->id * 28));
        text = load_text(bot->damage);
        draw_text(text, 303, 3 + 14 + (bot->id * 28));
        text = load_text(bot->energy);
        draw_text(text, 338, 3 + 14 + (bot->id * 28));
        text = load_text(bot->shield);
        draw_text(text, 373, 3 + 14 + (bot->id * 28));
    }
}

static void display( RW_Battle *b ) {
    RW_Active_Robot *bot;
    RW_Shot *shot;
    RW_Robot_Iter ri;
    RW_Shot_Iter si;
    SDL_FillRect(screen, NULL, 0xFFFFFFFF);
    RW_Reset_Shot_Iter(b, &si);
    while( (shot = RW_Shot_Next(&si)) ) {
        switch( shot->type ) {
            case shot_bullet:
            case shot_explosive:
            case shot_missile:
                draw_sprite(bullet, shot->x - 3, shot->y - 3);
                break;
            case shot_explosion:
                if(shot->timer == 3) {
                    draw_sprite(explosion12, shot->x - 12, shot->y - 12);
                }
                else if(shot->timer == 2) {
                    draw_sprite(explosion24, shot->x - 24, shot->y - 24);
                }
                else {
                    draw_sprite(explosion36, shot->x - 36, shot->y - 36);
                }
                break;
            case shot_hellbore:
                draw_sprite(hellbore, shot->x - 4, shot->y - 4);
                break;
            case shot_stunner:
                draw_sprite(stunner, shot->x - 4, shot->y - 4);
                break;
            default:
            case shot_mine:
            case shot_nuke:
                break;
        }
    }
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next(&ri)) ) {
        draw_sprite(robo[bot->id], bot->regs[reg_x] - 9, bot->regs[reg_y] - 9);
    }
    display_scores(b);
    SDL_Flip(screen);
}

static int handle_error( RW_Active_Robot* bot, enum RW_Error e, int val ) {
    fprintf(stdout, "Bot %d: ", bot->id);
    switch( e ) {
        case error_eof:
            fprintf(stdout, "Error: End of file reached! %d\n", val);
            break;
        case error_stack_uf:
            fprintf(stdout, "Error: Stack underflow!\n");
            break;
        case error_stack_of:
            fprintf(stdout, "Error: Stack overflow!\n");
            break;
        case error_end:
            fprintf(stdout, "\"End\" instruction executed!\n");
            break;
        case error_out_of_range:
            fprintf(stdout, "Error: Vector access out of range!\n");
            break;
        case error_unknown_op:
            fprintf(stdout, "Error: Unknown instruction! %d\n", val);
            break;
        case error_debug:
            fprintf(stdout, "DEBUG: %d\n", val);
            break;
    }
    return 0;
}

int run_chronon( RW_Battle *b ) {
    SDL_Event event;
    SDL_Delay(1000 / fps); /* 30 FPS */
    while( SDL_PollEvent(&event) ) {
        if( event.type == SDL_KEYDOWN && !keyflag ) {
            keyflag = !keyflag;
            switch(event.key.keysym.sym) {
                case SDLK_q:
                    return 1;
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
        else if( event.type == SDL_QUIT ) {
            return 1;
        }
    }
    display(b);
    return 0;
}

void after_fight( RW_Battle *b ) {
    RW_Active_Robot *bot;
    RW_Robot_Iter ri;
    fprintf(stdout, "Result of battle after %d chronons:\n", b->chronon);
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next_Raw(&ri)) ) {
        fprintf(stdout, "%s: %d points\n", RW_Get_Robot_Name(bot), bot->score);
    }
}

int main( int argc, char **argv ) {
    int i;
    RW_Battle *b;
    RW_Robot **bots;
    if( argc < 2 ) {
        fprintf(stdout, "Usage: %s robot [robot]+\n", argv[0]);
        return 0;
    }
    RW_Set_Base_Dir(argv[0]);
    bots = (RW_Robot**)malloc(sizeof(RW_Robot*) * (argc - 1));
    for( i = 1; i < argc; i++ ) {
        bots[i-1] = (RW_Robot*)malloc(sizeof(RW_Robot));
        bots[i-1] = RW_Read_Robot(argv[i]);
        bot_count++;
    }
    RW_Reset_Scores();
    b = RW_New_Battle();
    RW_Set_Error_Callback(b, handle_error);
    if( SDL_Init(SDL_INIT_TIMER) ) {
        return -1;
    }
    if( SDL_InitSubSystem(SDL_INIT_VIDEO) ) {
        return -1;
    }
    if( TTF_Init() ) {
        return -1;
    }
    screen = SDL_SetVideoMode(400, 300, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_WM_SetCaption("New RoboWar", "New RoboWar");
    bullet =      load_texture("bullet.png");
    robo[0] =     load_texture("robot.png");
    robo[1] =     load_texture("robot2.png");
    robo[2] =     load_texture("robot3.png");
    robo[3] =     load_texture("robot4.png");
    robo[4] =     load_texture("robot5.png");
    robo[5] =     load_texture("robot6.png");
    explosion36 = load_texture("explosion.png");
    explosion24 = load_texture("explosion24.png");
    explosion12 = load_texture("explosion12.png");
    stunner =     load_texture("stunner.png");
    hellbore =    load_texture("hellbore.png");
    font = load_font("DejaVuSans.ttf", 12);
    RW_Run_Duels(bots, bot_count, 1, b, run_chronon, after_fight);
    RW_Run_Groups(bots, bot_count, 1, b, run_chronon, after_fight);
    fprintf(stdout, "Score totals:\n");
    for( i = 0; i < bot_count; i++ ) {
        fprintf(stdout, "%s: %d / %d points\n", bots[i]->name, bots[i]->duel_score,
            bots[i]->group_score);
    }
    RW_Free_Battle(b);
    SDL_Quit();
    return 0;
}
