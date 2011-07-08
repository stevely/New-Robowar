/*
 * battlehandler.h
 *
 * By Steven Smith
 */
#ifndef BATTLEHANDLER_H_
#define BATTLEHANDLER_H_

#include "robocode.h"
#include "robotfile.h"
#include "robomath.h"

/* Battle handler structs */

typedef struct {
    size_t code_size;
    RW_Hardware_Spec hardware;
    unsigned int score;
    RW_Robo_Op *code;
} RW_Robot;

typedef struct RW_Robot_List {
    RW_Robot *bot;
    struct RW_Robot_List *next;
} RW_Robot_List;

typedef struct {
    RW_Robot *robot;
    int energy;
    int energy_cap;
    int damage;
    int damage_cap;
    int shield;
    int shield_cap;
    unsigned int id;
    int regs[512];
    int active;
    int stunned;
    int weakened_shields;
    unsigned int code_loc;
    unsigned int stack[50];
    int stack_loc;
    int vector[100];
} RW_Active_Robot;

/* Projectile structs */

enum RW_Shot_Type {
    shot_bullet,
    shot_explosion,
    shot_explosive,
    shot_hellbore,
    shot_missile,
    shot_mine,
    shot_nuke,
    shot_stunner
};

typedef struct {
    enum RW_Shot_Type type;
    int x;
    int y;
    int dx;
    int dy;
    int active;
    int range;
    int timer;
    int power;
    RW_Active_Robot *owner;
} RW_Shot;

typedef struct RW_Shot_Buffer {
    unsigned int length;
    RW_Shot *buf;
    struct RW_Shot_Buffer *next;
} RW_Shot_Buffer;

typedef struct {
    RW_Shot_Buffer *shots;
    RW_Shot_Buffer *bores;
} RW_Shot_State;

/* Code execution structs */

enum RW_Event_Type {
    event_movex,
    event_movey,
    event_speedx,
    event_speedy,
    event_bullet,
    event_explosive,
    event_stunner,
    event_hellbore,
    event_mine,
    event_nuke,
    event_missile,
    event_shield
};

enum RW_Error {
    error_eof,
    error_stack_uf,
    error_stack_of,
    error_end,
    error_out_of_range,
    error_unknown_op,
    error_debug
};

typedef struct {
    enum RW_Event_Type type;
    RW_Active_Robot *bot;
    int aim;
    int value;
} RW_Event;

typedef struct RW_Event_Queue {
    RW_Event *buf;
    int size;
    int loc;
    struct RW_Event_Queue *next;
} RW_Event_Queue;

/* Battle struct */

typedef struct {
    RW_Active_Robot bots[6];
    unsigned int score[6];
    unsigned int bot_count;
    unsigned int chronon;
    int hitmatrix[6][6];
    RW_Shot_State shots;
    RW_Event_Queue *queue;
    int (*err_fn)(RW_Active_Robot*, enum RW_Error, int val);
} RW_Battle;

/* Robot iterator */

typedef struct {
    RW_Battle *b;
    unsigned int i;
    RW_Active_Robot *s;
} RW_Robot_Iter;

/* Shot iterator */

typedef struct {
    RW_Battle *b;
    RW_Shot_Buffer *current_buf;
    unsigned int shot_index;
    int done_with_hellbores;
    unsigned int shot_counter;
} RW_Shot_Iter;

/* battlehandler.c code */

RW_Robot * RW_Read_Robot( char *fname );

void RW_Reset_Scores();

void RW_Setup_Battle( RW_Battle *b, RW_Robot **bots, size_t count );

void RW_Setup_Duel( RW_Battle *b, RW_Robot *b1, RW_Robot *b2 );

RW_Battle * RW_New_Battle();

void RW_Set_Error_Callback( RW_Battle *b, int (*fn)(RW_Active_Robot*, enum RW_Error, int val) );

void RW_Reset_Robot_Iter( RW_Battle *b, RW_Robot_Iter *i, RW_Active_Robot *bot );

RW_Active_Robot * RW_Robot_Next( RW_Robot_Iter *i );

RW_Active_Robot * RW_Robot_Next_Raw( RW_Robot_Iter *i );

int RW_Alive_Robots( RW_Battle *b );

int RW_Run_Chronon( RW_Battle *b );

/* projectiles.c code */

void RW_New_Shot( RW_Battle *b, RW_Active_Robot *bot, enum RW_Shot_Type type, int aim, int power );

int RW_Handle_Shot_Hit( RW_Battle *b, RW_Active_Robot *bot, RW_Shot *shot );

void RW_Shot_Cleanup( RW_Shot *shot );

void RW_Reset_Shot_Iter( RW_Battle *b, RW_Shot_Iter *si );

RW_Shot * RW_Shot_Next( RW_Shot_Iter *si );

/* codeexecution.c code */

void RW_Handle_Events( RW_Battle *b );

void RW_Run_Code( RW_Battle *b, RW_Active_Robot *bot );

#endif
