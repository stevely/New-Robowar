/*
 * battlehandler.c
 *
 * By Steven Smith
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "battlehandler.h"

static RW_Robot_List *bots = NULL;
static unsigned int bot_count = 0;

RW_Robot * RW_Read_Robot( char *fname ) {
    FILE *fp;
    RW_Robo_Op *code;
    unsigned int code_size;
    int c;
    RW_Robot_List *new_entry, *rl;
    RW_Robot *new_bot;
    RW_Robot_File rf;
    /* Step 1: Grab data from file */
    fp = fopen(fname, "rb");
    if( !fp ) {
        return NULL;
    }
    fread(&rf, sizeof(RW_Robot_File), 1, fp);
    fread(&code_size, sizeof(unsigned int), 1, fp);
    while((c = fgetc(fp)) != 0 && c != EOF);
    code = (RW_Robo_Op*)malloc(sizeof(RW_Robo_Op) * code_size);
    fread(code, sizeof(RW_Robo_Op), code_size, fp);
    fclose(fp);
    if( !code ) {
        return NULL;
    }
    /* Step 2: Move data to new robot entry */
    new_entry = (RW_Robot_List*)malloc(sizeof(RW_Robot_List));
    new_bot = (RW_Robot*)malloc(sizeof(RW_Robot));
    new_bot->code_size = code_size;
    new_bot->hardware = rf.hardware;
    new_bot->score = 0;
    new_bot->code = code;
    new_entry->bot = new_bot;
    new_entry->next = NULL;
    /* Step 3: Insert new robot into robot list */
    if( bots == NULL ) {
        bots = new_entry;
    }
    else {
        rl = bots;
        while( rl->next != NULL ) {
            rl = rl->next;
        }
        rl->next = new_entry;
    }
    bot_count++;
    return new_bot;
}

void RW_Reset_Scores() {
    RW_Robot_List *bot;
    bot = bots;
    while( bot ) {
        bot->bot->score = 0;
        bot = bot->next;
    }
}

static int valid_placement( RW_Battle *b, int i ) {
    int j, dist, dist_x, dist_y;
    for( j = 0; j < i - 1; j++ ) {
        /* dist > sqrt( (x1 - x2)^2 + (y1 - y2)^2 )
           dist^2 > (x1 - x2)^2 + (y1 - y2)^2 */
        dist_x = b->bots[i].regs[reg_x] - b->bots[j].regs[reg_x];
        dist_x *= dist_x;
        dist_y = b->bots[i].regs[reg_y] - b->bots[j].regs[reg_y];
        dist_y *= dist_y;
        dist = dist_x + dist_y;
        if( dist > 50*50 ) { /* Minimum 50 range */
            return 0;
        }
    }
    return 1;
}

static void reset_robot( RW_Active_Robot *bot ) {
    int i;
    if( !bot ) {
        return;
    }
    /* Set caps */
    /* Energy */
    switch( bot->robot->hardware.energy ) {
        case 0:
            bot->energy_cap = 50;
            break;
        case 1:
            bot->energy_cap = 100;
            break;
        case 2:
            bot->energy_cap = 150;
            break;
        default:
            bot->energy_cap = 0;
            break;
    }
    /* Damage */
    switch( bot->robot->hardware.damage ) {
        case 0:
            bot->damage_cap = 250;
            break;
        case 1:
            bot->damage_cap = 400;
            break;
        case 2:
            bot->damage_cap = 600;
            break;
        default:
            bot->damage_cap = 0;
            break;
    }
    /* Shield */
    switch( bot->robot->hardware.shield ) {
        case 0:
            bot->shield_cap = 0;
            break;
        case 1:
            bot->shield_cap = 50;
            break;
        case 2:
            bot->shield_cap = 100;
            break;
        default:
            bot->shield_cap = 0;
            break;
    }
    bot->energy = bot->energy_cap;
    bot->damage = bot->damage_cap;
    bot->shield = 0;
    bot->code_loc = 0;
    bot->stack_loc = 0;
    bot->active = 1;
    bot->stunned = 0;
    bot->weakened_shields = 0;
    /* We don't need to clear the stack because the old stack values can't
       be referenced once stack_loc gets cleared. We do need to clear the
       vector array, however, because it allows arbitrary references. */
    for( i = 0; i < 100; i++ ) {
        bot->vector[i] = 0;
    }
    /* We also need to clear the registers */
    for( i = 0; i < 512; i++ ) {
        bot->regs[i] = 0;
    }
}

void RW_Setup_Battle( RW_Battle *b, RW_Robot **bots, size_t count ) {
    size_t i, j;
    /* Reset scores, reset bots, place bots, and count bots */
    for( i = 0; i < count; i++ ) {
        b->bots[i].robot = bots[i];
        /* Reset scores */
        b->score[i] = 0;
        /* Reset bot */
        reset_robot(&(b->bots[i]));
        b->bots[i].id = i;
        b->bots[i].regs[reg_id] = i;
        /* Place randomly and check legality */
        do {
            b->bots[i].regs[reg_x] = (rand() % 201) + 50; /* Range: [50,250] */
            b->bots[i].regs[reg_y] = (rand() % 201) + 50;
        } while( !valid_placement(b, i) );
        /* Reset hit matrix */
        for( j = 0; j < 6; j++ ) {
            b->hitmatrix[i][j] = 0;
        }
    }
    for( ; i < 6; i++ ) {
        b->bots[i].robot = NULL;
        /* Reset hit matrix, part deux */
        for( j = 0; j < 6; j++ ) {
            b->hitmatrix[i][j] = 0;
        }
    }
    b->bot_count = count;
    b->chronon = 0;
}

/*
 * Convenience function.
 */
void RW_Setup_Duel( RW_Battle *b, RW_Robot *b1, RW_Robot *b2 ) {
    RW_Robot *bots[2] = {b1, b2};
    RW_Setup_Battle(b, bots, 2);
}

RW_Battle * RW_New_Battle() {
    RW_Battle *b;
    b = (RW_Battle*)malloc(sizeof(RW_Battle));
    b->queue = NULL;
    b->shots.shots = NULL;
    b->shots.bores = NULL;
    return b;
}

void RW_Reset_Robot_Iter( RW_Battle *b, RW_Robot_Iter *i, RW_Active_Robot *bot ) {
    i->b = b;
    i->i = 0;
    i->s = bot;
}

RW_Active_Robot * RW_Robot_Next( RW_Robot_Iter *i ) {
    if( i && i->b && i->i < i->b->bot_count ) {
        if( (i->s && i->s->id == i->b->bots[i->i].id) || !i->b->bots[i->i].active ) {
            i->i++;
            return RW_Robot_Next(i);
        }
        else {
            return &(i->b->bots[i->i++]);
        }
    }
    else {
        return NULL;
    }
}

RW_Active_Robot * RW_Robot_Next_Raw( RW_Robot_Iter *i ) {
    if( i && i->b && i->i < i->b->bot_count ) {
        return &(i->b->bots[i->i++]);
    }
    else {
        return NULL;
    }
}

int RW_Alive_Robots( RW_Battle *b ) {
    int count;
    RW_Robot_Iter i;
    RW_Active_Robot *bot;
    count = 0;
    RW_Reset_Robot_Iter(b, &i, NULL);
    while( (bot = RW_Robot_Next(&i)) ) {
        count++;
    }
    return count;
}

int RW_Run_Chronon( RW_Battle *b ) {
    int i, j, x, y, radius;
    RW_Shot *shot;
    RW_Active_Robot *bot1, *bot2;
    RW_Robot_Iter iter1, iter2, *i1, *i2;
    RW_Shot_Iter shot_iter, *si;
    i1 = &iter1;
    i2 = &iter2;
    si = &shot_iter;
    /* MAIN EVENT LOOP */
    /* Phase 1 */
    RW_Reset_Robot_Iter(b, i1, NULL);
    while( (bot1 = RW_Robot_Next(i1)) ) {
        if( bot1->active ) {
            x = bot1->regs[reg_x];
            y = bot1->regs[reg_y];
            /* Wall check */
            if( x < 10 || y < 10 || x > 290 || y > 290 ) {
                bot1->damage--;
            }
            /* Dead bot check */
            if( bot1->damage <= 0 ) {
                bot1->active = 0;
                RW_Reset_Robot_Iter(b, i2, bot1);
                while( (bot2 = RW_Robot_Next(i2)) ) {
                    /* Out-living point */
                    if( bot2->damage > 0 ) {
                        b->score[bot2->id]++;
                    }
                    /* Kill point */
                    if( b->hitmatrix[bot1->id][bot2->id] ) {
                        b->score[bot2->id]++;
                        bot2->regs[reg_kills]++;
                    }
                }
            }
            else {
                bot1->weakened_shields = 0;
                if( bot1->stunned ) {
                    bot1->stunned--;
                }
                else {
                    /* Award energy */
                    bot1->energy += 2;
                    if( bot1->energy > bot1->energy_cap ) {
                        bot1->energy = bot1->energy_cap;
                    }
                    /* Shield drain */
                    if( bot1->shield && (bot1->shield > bot1->shield_cap) || b->chronon % 2 ) {
                        bot1->shield--;
                    }
                    if( bot1->energy >= 0 ) {
                        x = bot1->regs[reg_x] + bot1->regs[reg_speedx];
                        y = bot1->regs[reg_y] + bot1->regs[reg_speedy];
                        if( x < 0 ) {
                            x = 0;
                        }
                        if( x > 300 ) {
                            x = 300;
                        }
                        if( y < 0 ) {
                            y = 0;
                        }
                        if( y > 300 ) {
                            y = 300;
                        }
                        bot1->regs[reg_x] = x;
                        bot1->regs[reg_y] = y;
                        RW_Run_Code(b, bot1);
                    }
                }
            }
        }
    }
    /* Phase 2 */
    RW_Handle_Events(b);
    /* Phase 3 */
    for( i = 0; i < 6; i++ ) { /* Reset hit matrix */
        for( j = 0; j < 6; j++ ) {
            b->hitmatrix[i][j] = 0;
        }
    }
    RW_Reset_Shot_Iter(b, si);
    while( (shot = RW_Shot_Next(si)) ) {
        /* Move shot */
        shot->x += shot->dx;
        shot->y += shot->dy;
        if( shot->timer ) {
            shot->timer--;
        }
        else {
            /* Calculate hits */
            RW_Reset_Robot_Iter(b, i1, NULL);
            while( (bot1 = RW_Robot_Next(i1)) ) {
                x = shot->x - bot1->regs[reg_x];
                y = shot->y - bot1->regs[reg_y];
                radius = 10 + shot->range;
                if( x*x + y*y < radius*radius ) {
                    /* Hit */
                    if( RW_Handle_Shot_Hit(b, bot1, shot) ) {
                        /* bot1 was hit by shot->owner */
                        b->hitmatrix[bot1->id][shot->owner->id] = 1;
                    }
                }
            }
        }
        /* Check bounds */
        if( shot->x < -30 || shot->y < -30 || shot->x > 330 || shot->y > 330 ) {
            shot->active = 0;
        }
    }
    b->chronon++;
    if( b->chronon == 1500 ) {
        /* Round timeout */
        RW_Reset_Robot_Iter(b, i1, NULL);
        while( (bot1 = RW_Robot_Next(i1)) ) {
            if( bot1->damage > 0 ) {
                /* Point for surviving the round */
                b->score[bot1->id]++;
            }
        }
        return 0;
    }
    return RW_Alive_Robots(b) > 1;
}
