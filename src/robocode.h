/*
 * robocode.h
 *
 * By Steven Smith
 */

#ifndef ROBOCODE_H_
#define ROBOCODE_H_

#include <math.h>

/*
 * The RoboWar operator type. 32-bits long.
 */
typedef int RW_Robo_Op;

#define opcode_mask 0xF8000000 /* 5-bits long */
#define opreg1_mask 0x07FC0000 /* 9-bits long */
#define opreg2_mask 0x0003FE00 /* 9-bits long */
#define opreg3_mask 0x000001FF /* 9-bits long */
#define opimme_mask 0x07FFFFFF /* 27-bits long */
#define opiden_mask opreg3_mask /* 9-bits long */

/*
 * Operators for the opcode field. Most operators have at least 1 unused
 * register, so an ident field is used in the case that space is available,
 * allowing us a much shorter opcode field.
 */
enum op_code {
    op_nop      =  0,
    op_ne       =  1,
    op_eq       =  2,
    op_lt       =  3,
    op_gt       =  4,
    op_add      =  5,
    op_sub      =  6,
    op_mul      =  7,
    op_div      =  8,
    op_mod      =  9,
    op_and      = 10,
    op_or       = 11,
    op_xor      = 12,
    op_max      = 13,
    op_min      = 14,
    op_dist     = 15,
    op_sin      = 16,
    op_cos      = 17,
    op_tan      = 18,
    op_arcsin   = 19,
    op_arccos   = 20,
    op_arctan   = 21,
    op_two_reg  = 22,
    op_one_reg  = 23,
    op_zero_reg = 24,
    op_call     = 25,
    op_jump     = 26,
    op_mova     = 27,
    op_movb     = 28,
    op_push     = 29
};

/*
 * Possible ident values for operators using the two-register opcode.
 */
enum ident_tworeg {
    op_mov      =  0,
    op_not      =  1,
    op_chs      =  2,
    op_abs      =  3,
    op_sqrt     =  4,
    op_vrecall  =  5,
    op_vstore   =  6,
    op_setparam =  7
};

/*
 * Possible ident values for operators using the one-register opcode.
 */
enum ident_onereg {
    op_test     =  0,
    op_icon     =  1,
    op_sound    =  2,
    op_roll     =  3,
    op_random   =  4,
    op_print    =  5,
    op_peek     =  6,
    op_pop      =  7,
    op_pushr    =  8
};

/*
 * Possible ident values for operators using the zero-register opcode.
 */
/* NOTE: For legacy mode, the entirety of RoboTalk operators will need to be
   added here. */
enum ident_zeroreg {
    op_beep     =  0,
    op_debug    =  1,
    op_drop     =  2,
    op_dropall  =  3,
    op_dup      =  4,
    op_end      =  5,
    op_recall   =  6,
    op_return   =  7,
    op_store    =  8,
    op_swap     =  9,
    op_sync     = 10
};

enum special_reg {
    reg_aim       = 511,
    reg_bullet    = 510,
    reg_channel   = 509,
    reg_chronon   = 508,
    reg_collision = 507,
    reg_damage    = 506,
    reg_energy    = 505,
    reg_fire      = 504,
    reg_friend    = 503,
    reg_hellbore  = 502,
    reg_history   = 501,
    reg_id        = 500,
    reg_kills     = 499,
    reg_look      = 498,
    reg_mine      = 497,
    reg_missile   = 496,
    reg_movex     = 495,
    reg_movey     = 494,
    reg_nuke      = 493,
    reg_probe     = 492,
    reg_radar     = 491,
    reg_range     = 490,
    reg_robots    = 489,
    reg_scan      = 488,
    reg_shield    = 487,
    reg_signal    = 486,
    reg_speedx    = 485,
    reg_speedy    = 484,
    reg_stunner   = 483,
    reg_teammates = 482,
    reg_wall      = 481,
    reg_x         = 480,
    reg_y         = 479,
    reg_a         = 478,
    reg_b         = 477
};

/*
 * Helper macros
 */
#define get_opcode(op) (op >> 27)
#define get_opreg1(op) ((op >> 18) & opreg1_mask)
#define get_opreg2(op) ((op >> 9) & opreg2_mask)
#define get_opreg3(op) (op & opreg3_mask)
#define get_opimmediate(op) (op & opiden_mask)

#define fivebitmask 0x0000001F
#define ninebitmask 0x000001FF

/*
 * Given the original operator and 5 ints (the actual names, not pointers), this
 * macro fills the variables with the opcode, reg1, reg2, reg3, and the
 * immediate value. The identifier for [0,2]-register ops is in reg3.
 * Remember that this is not a function, and should not be used like a function.
 * This macro should only be used to decode operators.
 */
#define decode_op(op, opcode, reg1, reg2, reg3, immed) \
    do { \
        immed = reg3 = op; \
        reg2 = reg3 >> 9; \
        reg1 = reg2 >> 9; \
        opcode = reg1 >> 9; \
        immed &= opimme_mask; \
        reg1 &= ninebitmask; \
        reg2 &= ninebitmask; \
        reg3 &= ninebitmask; \
        opcode &= fivebitmask; \
    } while(0);

#define encode_op_regs(op, opcode, reg1, reg2, reg3) \
    do { \
        op = ((opcode << 27) & opcode_mask) \
           | ((reg1 << 18) & opreg1_mask) \
           | ((reg2 << 9) & opreg2_mask) \
           | (reg3 & opreg3_mask); \
    } while(0);

#define encode_op_imme(op, opcode, immed) \
    do { \
        op = ((opcode << 27) & opcode_mask) \
           | (immed & opimme_mask); \
    } while(0);

#define update_op_imme(op, new_immed) \
    do { \
        op = (op & opcode_mask) \
           | (new_immed & opimme_mask); \
    } while(0);

#define sign_extend(i) (((i) + 0x7c000000) ^ 0x7c000000)

#define degree_convert(n) ((int)((360/(2*M_PI))*(n)))

#define angle_convert(n) ((450+(degree_convert(n)))%360)

#endif
