/*
 * battletest.c
 *
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include "battlehandler.h"

static int battle_count = 0;
static unsigned long total_chronons = 0;

void print_battle_results( RW_Battle *b ) {
    RW_Active_Robot *bot;
    RW_Robot_Iter ri;
    fprintf(stdout, "Result of battle after %d chronons:\n", b->chronon);
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next_Raw(&ri)) ) {
        fprintf(stdout, "%s: %d points\n", bot->robot->name, bot->score);
    }
}

void inc_b_count( RW_Battle *b ) {
    battle_count++;
    total_chronons += b->chronon;
}

int main( int argc, char **argv ) {
    int i;
    RW_Battle *b;
    RW_Robot **bots;
    int bot_count = 0;
    if( argc < 2 ) {
        fprintf(stdout, "Usage: %s robot [robot]+\n", argv[0]);
        return 0;
    }
    bots = (RW_Robot**)malloc(sizeof(RW_Robot*) * (argc - 1));
    for( i = 1; i < argc; i++ ) {
        bots[i-1] = (RW_Robot*)malloc(sizeof(RW_Robot));
        bots[i-1] = RW_Read_Robot(argv[i]);
        bot_count++;
    }
    RW_Reset_Scores();
    b = RW_New_Battle();
    RW_Run_Duels(bots, bot_count, 10, b, NULL, inc_b_count);
    RW_Run_Groups(bots, bot_count, 1, b, NULL, inc_b_count);
    fprintf(stdout, "%d battles run, totalling %lu chronons\n", battle_count, total_chronons);
    fprintf(stdout, "==============\n");
    fprintf(stdout, " Score totals:\n");
    fprintf(stdout, "==============\n");
    for( i = 0; i < bot_count; i++ ) {
        fprintf(stdout, "%s: %d, %d, %d\n", bots[i]->name, bots[i]->duel_score,
            bots[i]->group_score, bots[i]->duel_score + bots[i]->group_score);
    }
    RW_Free_Battle(b);
    for( i = 0; i < bot_count; i++ ) {
        free(bots[i]->code);
        free(bots[i]->name);
        free(bots[i]);
    }
    free(bots);
    return 0;
}
