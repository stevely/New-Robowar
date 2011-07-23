/*
 * battletest.c
 *
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include "battlehandler.h"

void print_battle_results( RW_Battle *b ) {
    RW_Active_Robot *bot;
    RW_Robot_Iter ri;
    fprintf(stdout, "Result of battle after %d chronons:\n", b->chronon);
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next_Raw(&ri)) ) {
        fprintf(stdout, "%s: %d points\n", bot->robot->name, bot->score);
    }
}

void update_scores( RW_Battle *b, int duel ) {
    RW_Active_Robot *bot;
    RW_Robot_Iter ri;
    RW_Reset_Robot_Iter(b, &ri, NULL);
    while( (bot = RW_Robot_Next_Raw(&ri)) ) {
        if( duel ) {
            RW_Update_Duel_Score(bot);
        }
        else {
            RW_Update_Group_Score(bot);
        }
    }
}

void duels( RW_Robot **bots, int bot_count, RW_Battle *b ) {
    int i, j;
    fprintf(stdout, "Solo rounds:\n");
    for( i = 0; i < bot_count; i++ ) {
        for( j = i+1; j < bot_count; j++ ) {
            RW_Setup_Duel(b, bots[i], bots[j]);
            while( RW_Run_Chronon(b) );
            print_battle_results(b);
            update_scores(b, 1);
        }
    }
}

void groups( RW_Robot **bots, int bot_count, RW_Battle *b ) {
    int i, j, count = 0;
    int is[6] = {0, 1, 2, 3, 4, 5};
    RW_Robot *bs[6];
    RW_Robot *first = *bots;
    if( bot_count < 3 ) {
        return;
    }
    if( bot_count <= 6 ) {
        for( i = 0; i < 6; i++ ) {
            bs[i] = bots[is[i]];
        }
        RW_Setup_Battle(b, bs, 6);
        while( RW_Run_Chronon(b) );
        print_battle_results(b);
        update_scores(b, 0);
        return;
    }
    while( is[5] < bot_count ) {
        for( i = 0; i < 6; i++ ) {
            bs[i] = bots[is[i]];
        }
        if(bs[0] == first ) {
            count++;
        }
        RW_Setup_Battle(b, bs, 6);
        while( RW_Run_Chronon(b) );
        print_battle_results(b);
        update_scores(b, 0);
        i = 0;
        while( 1 ) {
            if( i == 5 ) {
                is[5]++;
                break;
            }
            else {
                if( is[i] + 1 == is[i+1] ) {
                    i++;
                    for( j = 0; j < i; j++ ) {
                        is[j] = j;
                    }
                }
                else {
                    is[i]++;
                    for( j = 0; j < i; j++ ) {
                        is[j] = j;
                    }
                    break;
                }
            }
        }
    }
    fprintf(stdout, "Each bots played %d group rounds.\n", count);
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
    duels(bots, bot_count, b);
    groups(bots, bot_count, b);
    fprintf(stdout, "==============\n");
    fprintf(stdout, " Score totals:\n");
    fprintf(stdout, "==============\n");
    for( i = 0; i < bot_count; i++ ) {
        fprintf(stdout, "%s: %d, %d, %d\n", bots[i]->name, bots[i]->duel_score,
            bots[i]->group_score, bots[i]->duel_score + bots[i]->group_score);
    }
    RW_Free_Battle(b);
    return 0;
}
