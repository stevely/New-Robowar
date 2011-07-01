/*
 * battletest.c
 *
 * By Steven Smith
 */

#include <stdio.h>
#include "battlehandler.h"

int main( int argc, char **argv ) {
    int i;
    RW_Battle *b;
    RW_Robot *bots[6];
    RW_Active_Robot *bot;
    RW_Robot_Iter ri;
    size_t bot_count = 0;
    if( argc < 2 ) {
        fprintf(stdout, "Usage: %s robot [robot]+\n", argv[0]);
        return 0;
    }
    for( i = 1; i < argc; i++ ) {
        bots[i-1] = RW_Read_Robot(argv[i]);
        bot_count++;
    }
    RW_Reset_Scores();
    b = RW_New_Battle();
    RW_Setup_Battle(b, bots, bot_count);
    while( RW_Run_Chronon(b) );
    fprintf(stdout, "Result of battle after %d chronons:\n", b->chronon);
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next_Raw(&ri)) ) {
        fprintf(stdout, "%d: %d points\n", bot->id, b->score[bot->id]);
    }
    return 0;
}
