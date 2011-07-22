/*
 * battletest.c
 *
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include "battlehandler.h"

int main( int argc, char **argv ) {
    int i, j;
    RW_Battle *b;
    RW_Robot **bots;
    RW_Active_Robot *bot;
    RW_Robot_Iter ri;
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
    for( i = 0; i < bot_count; i++ ) {
        for( j = i+1; j < bot_count; j++ ) {
            RW_Setup_Duel(b, bots[i], bots[j]);
            while( RW_Run_Chronon(b) );
            fprintf(stdout, "Result of battle after %d chronons:\n", b->chronon);
            RW_Reset_Robot_Iter(b, &ri, NULL);
            while( (bot = RW_Robot_Next_Raw(&ri)) ) {
                fprintf(stdout, "%s: %d points\n", bot->robot->name, b->score[bot->id]);
                bot->robot->score += b->score[bot->id];
            }
        }
    }
    fprintf(stdout, "Score totals:\n");
    for( i = 0; i < bot_count; i++ ) {
        fprintf(stdout, "%s: %d points\n", bots[i]->name, bots[i]->score);
    }
    RW_Free_Battle(b);
    return 0;
}
