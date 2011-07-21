/*
 * projectiles.c
 *
 * By Steven Smith
 */

#include <stdio.h>
#include <stdlib.h>
#include "battlehandler.h"

static RW_Shot * create_shot( RW_Shot_Buffer **buffer ) {
    unsigned int i;
    RW_Shot_Buffer *buf;
    buf = *buffer;
    if( !buf ) {
        *buffer = (RW_Shot_Buffer*)malloc(sizeof(RW_Shot_Buffer));
        (*buffer)->buf = (RW_Shot*)calloc(20, sizeof(RW_Shot));
        (*buffer)->next = NULL;
        (*buffer)->length = 20;
        buf = *buffer;
    }
    /* Only fill in the last buffer */
    while( buf->next ) {
        buf = buf->next;
    }
    /* Attempt to find available spot */
    for( i = 0; i < buf->length; i++ ) {
        if( !buf->buf[i].active ) {
            return &(buf->buf[i]);
        }
    }
    /* Buffer is full, create new buffer */
    buf->next = (RW_Shot_Buffer*)malloc(sizeof(RW_Shot_Buffer));
    /* We need calloc here to ensure that new slots start with the active flag cleared */
    buf->next->buf = (RW_Shot*)calloc(buf->length * 2, sizeof(RW_Shot));
    buf->next->next = NULL;
    buf->next->length = buf->length * 2;
    return buf->next->buf;
}

void RW_New_Shot( RW_Battle *b, RW_Active_Robot *bot, enum RW_Shot_Type type, int aim, int power ) {
    RW_Shot *shot;
    int speed;
    if( type == shot_hellbore ) {
        shot = create_shot(&(b->shots.bores));
    }
    else {
        shot = create_shot(&(b->shots.shots));
    }
    shot->active = 1;
    shot->x = bot->regs[reg_x];
    shot->x_raw = shot->x << 8;
    shot->y = bot->regs[reg_y];
    shot->y_raw = shot->y << 8;
    shot->type = type;
    shot->owner = bot;
    switch(type) {
        case shot_bullet:
        case shot_explosive:
            speed = 12;
            shot->timer = 1;
            shot->range = 3;
            shot->power = power;
            break;
        case shot_explosion:
            speed = 0;
            shot->timer = 3;
            shot->range = 36;
            shot->power = power;
            break;
        case shot_hellbore:
            speed = power;
            shot->timer = 1;
            shot->range = 3;
            shot->power = 0;
            break;
        case shot_missile:
            speed = 5;
            shot->timer = 1;
            shot->range = 3;
            shot->power = power * 3 / 2;
            break;
        case shot_mine:
            speed = 0;
            shot->timer = 10;
            shot->range = 5;
            shot->power = power * 2;
            break;
        case shot_nuke:
            speed = 0;
            shot->timer = 10;
            shot->range = 50;
            shot->power = power * 2;
            break;
        case shot_stunner:
            speed = 14;
            shot->timer = 1;
            shot->range = 3;
            shot->power = power / 4;
            break;
        default:
            shot->active = 0;
            return;
    }
    if( speed ) {
        shot->dx = robo_sin(speed << 8, aim);
        shot->dy = robo_cos(speed << 8, aim);
    }
}

/*
 * Creates an explosion resulting from an exposive bullet hit.
 */
static void create_explosion( RW_Battle *b, RW_Active_Robot *bot, int x, int y, int power ) {
    RW_Shot *shot;
    shot = create_shot(&(b->shots.shots));
    shot->active = 1;
    shot->x = x;
    shot->y = y;
    shot->x_raw = x << 8;
    shot->y_raw = y << 8;
    shot->dx = 0;
    shot->dy = 0;
    shot->type = shot_explosion;
    shot->owner = bot;
    shot->power = power;
    shot->timer = 3;
    shot->range = 36;
}

int RW_Handle_Shot_Hit( RW_Battle *b, RW_Active_Robot *bot, RW_Shot *shot ) {
    int damage_dealt = 0;
    switch( shot->type ) {
        case shot_explosive:
            /* Prevents multiple explosion spawns */
            if( shot->active && shot->power > 1 ) {
                create_explosion(b, shot->owner, shot->x, shot->y, shot->power/2);
            } /* Fall through */
        default:
        case shot_bullet:
        case shot_explosion:
        case shot_nuke:
        case shot_mine:
        case shot_missile:
            if( bot->shield <= 0 ) {
                bot->damage -= shot->power;
            }
            else {
                if( bot->weakened_shields ) {
                    if( bot->shield > bot->shield_cap ) {
                        if( shot->power * 3 >= (bot->shield - bot->shield_cap) / 2 ) {
                            bot->shield = bot->shield_cap -
                                (shot->power * 3 - (bot->shield - bot->shield_cap) / 2);
                        }
                        else {
                            bot->shield -= shot->power * 6;
                        }
                    }
                    else {
                        bot->shield -= shot->power * 3;
                    }
                    if( bot->shield < 0 ) {
                        bot->damage += bot->shield / 3;
                        bot->shield = 0;
                    }
                }
                else {
                    if( bot->shield > bot->shield_cap ) {
                        if( shot->power >= (bot->shield - bot->shield_cap) / 2 ) {
                            bot->shield = bot->shield_cap -
                                (shot->power - (bot->shield - bot->shield_cap) / 2);
                        }
                        else {
                            bot->shield -= shot->power * 2;
                        }
                    }
                    else {
                        bot->shield -= shot->power;
                    }
                    if( bot->shield < 0 ) {
                        bot->damage += bot->shield;
                        bot->shield = 0;
                    }
                }
            }
            damage_dealt = 1;
            break;
        case shot_stunner:
            bot->stunned += shot->power;
            break;
        case shot_hellbore:
            bot->weakened_shields = 1;
            break;
    }
    shot->active = 0;
    return damage_dealt;
}

void RW_Shot_Update( RW_Shot *shot ) {
    shot->x_raw += shot->dx;
    shot->x = shot->x_raw >> 8;
    shot->y_raw += shot->dy;
    shot->y = shot->y_raw >> 8;
}

void RW_Shot_Cleanup( RW_Shot *shot ) {
    /* Check bounds */
    if( shot->x < -30 || shot->y < -30 || shot->x > 330 || shot->y > 330 ) {
        shot->active = 0;
    }
    if( !shot->timer && (shot->type == shot_explosion || shot->type == shot_nuke) ) {
        shot->active = 0;
    }
}

void RW_Reset_Shot_Iter( RW_Battle *b, RW_Shot_Iter *si ) {
    si->b = b;
    si->shot_index = 0;
    si->current_buf = b->shots.bores;
    si->shot_counter = 0;
    si->done_with_hellbores = 0;
}

RW_Shot * RW_Shot_Next( RW_Shot_Iter *si ) {
    if( !si->current_buf ) {
        if( si->done_with_hellbores ) {
            return NULL;
        }
        else {
            si->done_with_hellbores = 1;
            si->current_buf = si->b->shots.shots;
            si->shot_index = 0;
            si->shot_counter = 0;
            return RW_Shot_Next(si);
        }
    }
    else {
        /* Look for an active shot in the current buffer */
        while( si->shot_index < si->current_buf->length ) {
            if( si->current_buf->buf[si->shot_index].active ) {
                si->shot_counter++;
                return &(si->current_buf->buf[si->shot_index++]);
            }
            else {
                si->shot_index++;
            }
        }
        /* Didn't find any more shots in current buffer, see if we can free it */
        /* Shot counter: If a buffer is the first, has no active shots in it,
           and its next pointer points to another buffer, we free the buffer
           and promote the next buffer to the head buffer. */
        if( si->current_buf->next && si->shot_counter == 0 ) {
            if( si->current_buf == si->b->shots.shots ) {
                si->current_buf = si->current_buf->next;
                free(si->b->shots.shots->buf);
                free(si->b->shots.shots);
                si->b->shots.shots = si->current_buf;
            }
            else if( si->current_buf == si->b->shots.bores ) {
                si->current_buf = si->current_buf->next;
                free(si->b->shots.bores->buf);
                free(si->b->shots.bores);
                si->b->shots.bores = si->current_buf;
            }
        }
        si->shot_index = 0;
        si->shot_counter = 0;
        si->current_buf = si->current_buf->next;
        return RW_Shot_Next(si);
    }
}

static void free_shot_buffer( RW_Shot_Buffer *b ) {
    if( b == NULL ) {
        return;
    }
    if( b->next ) {
        free_shot_buffer(b->next);
    }
    free(b->buf);
    free(b);
}

void RW_Free_Shot_State( RW_Shot_State s ) {
    if( s.shots ) {
        free_shot_buffer(s.shots);
    }
    if( s.bores ) {
        free_shot_buffer(s.bores);
    }
}
