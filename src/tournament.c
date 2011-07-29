/*
 * tournament.c
 *
 * By Steven Smith
 */

#include "battlehandler.h"

static void update_scores( RW_Battle *b, int duel ) {
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

void RW_Run_Duels( RW_Robot **bots, int bot_count, int rounds, RW_Battle *b,
    int (*each_chronon)(RW_Battle*), void (*after_battle)(RW_Battle*) ) {
    int i, j, k;
    if( bots == NULL || b == NULL ) {
        return;
    }
    for( i = 0; i < bot_count; i++ ) {
        for( j = i+1; j < bot_count; j++ ) {
            for( k = 0; k < rounds; k++ ) {
                RW_Setup_Duel(b, bots[i], bots[j]);
                if( each_chronon ) {
                    while( RW_Run_Chronon(b) ) {
                        if( each_chronon(b) ) {
                            return;
                        }
                    }
                }
                else {
                    while( RW_Run_Chronon(b) );
                }
                update_scores(b, 1);
                if( after_battle ) {
                    after_battle(b);
                }
            }
        }
    }
}

void RW_Run_Groups( RW_Robot **bots, int bot_count, int rounds, RW_Battle *b,
    int (*each_chronon)(RW_Battle*), void (*after_battle)(RW_Battle*) ) {
    int i, j, k, count;
    int is[6] = {0, 1, 2, 3, 4, 5};
    RW_Robot *bs[6];
    if( bots == NULL || b == NULL ) {
        return;
    }
    count = bot_count > 6 ? 6 : bot_count;
    /* Minimum 3 robots needed for group rounds */
    if( bot_count < 3 ) {
        return;
    }
    do {
        /* Move the current set of indices into the bs array */
        for( i = 0; i < 6; i++ ) {
            bs[i] = bots[is[i]];
        }
        for( k = 0; k < rounds; k++ ) {
            RW_Setup_Battle(b, bs, count);
            if( each_chronon ) {
                while( RW_Run_Chronon(b) ) {
                    if( each_chronon(b) ) {
                        return;
                    }
                }
            }
            else {
                while( RW_Run_Chronon(b) );
            }
            update_scores(b, 0);
            if( after_battle ) {
                after_battle(b);
            }
        }
        i = 0;
        /* Get next combination */
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
    } while( is[5] < bot_count );
}
