/*
 * codeexecution.c
 *
 * By Steven Smith
 */

#include <stdlib.h>
#include <math.h>
#include "battlehandler.h"

static void add_event( RW_Battle *b, enum RW_Event_Type type, RW_Active_Robot *bot, int value ) {
    RW_Event_Queue *s;
    /* Don't have any buffers yet */
    if( !b->queue ) {
        b->queue = (RW_Event_Queue*)malloc(sizeof(RW_Event_Queue));
        b->queue->size = 20;
        b->queue->loc = 0;
        b->queue->next = NULL;
        b->queue->buf = (RW_Event*)malloc(sizeof(RW_Event) * b->queue->size);
    }
    s = b->queue;
    /* Only add to last buffer */
    while( s->next ) {
        s = s->next;
    }
    /* Need to make a new buffer */
    if( s->loc == s->size - 1 ) {
        s->next = (RW_Event_Queue*)malloc(sizeof(RW_Event_Queue));
        s->next->size = s->size * 2;
        s->next->loc = 0;
        s->next->next = NULL;
        s->next->buf = (RW_Event*)malloc(sizeof(RW_Event) * s->next->size);
        s = s->next;
    }
    s->buf[s->loc].type = type;
    s->buf[s->loc].bot = bot;
    s->buf[s->loc].aim = bot->regs[reg_aim];
    s->buf[s->loc].value = value;
    s->loc++;
}

static int get_energy_used( RW_Active_Robot *bot, int value ) {
    if( bot->robot->hardware.negenergy ) {
        if( value > bot->energy_cap ) {
            return bot->energy_cap;
        }
        else {
            return value;
        }
    }
    else {
        if( value > bot->energy ) {
            return bot->energy;
        }
        else {
            return value;
        }
    }
}

void RW_Handle_Events( RW_Battle *b ) {
    RW_Event_Queue *s;
    int i, x, y;
    int energy_used;
    enum RW_Shot_Type shot;
    /* Handle events */
    s = b->queue;
    while( s ) {
        for( i = 0; i < s->loc; i++ ) {
            /* I'm doing a bit of a hack here. Switch statements support
               break but not continue, so the break statements refer to the
               switch block while the continue statements refer to the for
               loop. This lets me do a bit of fancy flow control to reduce
               redundant code */
            switch( s->buf[i].type ) {
                case event_movex:
                    energy_used = get_energy_used(s->buf[i].bot, s->buf[i].value * 2);
                    s->buf[i].bot->energy -= energy_used;
                    if( s->buf[i].value > 0 ) {
                        x = s->buf[i].bot->regs[reg_x] + (energy_used / 2);
                        if( x > 300 ) {
                            x = 300;
                        }
                        s->buf[i].bot->regs[reg_x] = x;
                    }
                    else {
                        x = s->buf[i].bot->regs[reg_x] - (energy_used / 2);
                        if( x < 0 ) {
                            x = 0;
                        }
                        s->buf[i].bot->regs[reg_x] = x;
                    }
                    continue;
                case event_movey:
                    energy_used = get_energy_used(s->buf[i].bot, s->buf[i].value * 2);
                    s->buf[i].bot->energy -= energy_used;
                    if( s->buf[i].value > 0 ) {
                        y = s->buf[i].bot->regs[reg_y] + (energy_used / 2);
                        if( y > 300 ) {
                            y = 300;
                        }
                        s->buf[i].bot->regs[reg_y] = y;
                    }
                    else {
                        y = s->buf[i].bot->regs[reg_y] - (energy_used / 2);
                        if( y < 0 ) {
                            y = 0;
                        }
                        s->buf[i].bot->regs[reg_y] = y;
                    }
                    continue;
                case event_speedx:
                    if( s->buf[i].value > 20 ) {
                        s->buf[i].value = 20;
                    }
                    else if( s->buf[i].value < -20 ) {
                        s->buf[i].value = -20;
                    }
                    if( s->buf[i].bot->regs[reg_speedx] > s->buf[i].value ) {
                        energy_used = get_energy_used(s->buf[i].bot,
                            (s->buf[i].bot->regs[reg_speedx] - s->buf[i].value) * 2);
                        s->buf[i].bot->energy -= energy_used;
                        s->buf[i].bot->regs[reg_speedx] -= energy_used / 2;
                    }
                    else if( s->buf[i].bot->regs[reg_speedx] < s->buf[i].value ) {
                        energy_used = get_energy_used(s->buf[i].bot,
                            (s->buf[i].value - s->buf[i].bot->regs[reg_speedx]) * 2);
                        s->buf[i].bot->energy -= energy_used;
                        s->buf[i].bot->regs[reg_speedx] += energy_used / 2;
                    }
                    continue;
                case event_speedy:
                    if( s->buf[i].value > 20 ) {
                        s->buf[i].value = 20;
                    }
                    else if( s->buf[i].value < -20 ) {
                        s->buf[i].value = -20;
                    }
                    if( s->buf[i].bot->regs[reg_speedy] > s->buf[i].value ) {
                        energy_used = get_energy_used(s->buf[i].bot,
                            (s->buf[i].bot->regs[reg_speedy] - s->buf[i].value) * 2);
                        s->buf[i].bot->energy -= energy_used;
                        s->buf[i].bot->regs[reg_speedy] -= energy_used / 2;
                    }
                    else if( s->buf[i].bot->regs[reg_speedy] < s->buf[i].value ) {
                        energy_used = get_energy_used(s->buf[i].bot,
                            (s->buf[i].value - s->buf[i].bot->regs[reg_speedy]) * 2);
                        s->buf[i].bot->energy -= energy_used;
                        s->buf[i].bot->regs[reg_speedy] += energy_used / 2;
                    }
                    continue;
                case event_bullet:
                    shot = shot_bullet;
                    break;
                case event_explosive:
                    shot = shot_explosive;
                    break;
                case event_stunner:
                    shot = shot_stunner;
                    break;
                case event_hellbore:
                    shot = shot_hellbore;
                    break;
                case event_mine:
                    shot = shot_mine;
                    break;
                case event_nuke:
                    shot = shot_nuke;
                    break;
                case event_missile:
                    shot = shot_missile;
                    break;
                case event_shield:
                    if( s->buf[i].value < s->buf[i].bot->shield ) {
                        s->buf[i].bot->energy += s->buf[i].bot->shield - s->buf[i].value;
                        s->buf[i].bot->shield = s->buf[i].value;
                    }
                    else {
                        if( s->buf[i].bot->energy > 0 ) {
                            energy_used = get_energy_used(s->buf[i].bot, s->buf[i].value - s->buf[i].bot->shield);
                            s->buf[i].bot->energy -= energy_used;
                            s->buf[i].bot->shield += energy_used;
                        }
                    }
                    continue;
                default:
                    continue;
            }
            /* For shots */
            if( s->buf[i].bot->energy > 0 ) {
                energy_used = get_energy_used(s->buf[i].bot, s->buf[i].value);
                if( energy_used > 0 ) {
                    s->buf[i].bot->energy -= energy_used;
                    RW_New_Shot(b, s->buf[i].bot, shot, s->buf[i].aim, energy_used);
                }
            }
        }
        s = s->next;
    }
    /* Cleanup buffers */
    s = b->queue;
    while( s ) {
        if( s->next && s == b->queue ) {
            s = s->next;
            free(b->queue->buf);
            free(b->queue);
            b->queue = s;
        }
        else {
            s->loc = 0;
            s = s->next;
        }
    }
}

static void report_error( RW_Battle *b, RW_Active_Robot *bot, enum RW_Error err, int val ) {
    if( b->err_fn ) {
        b->err_fn(bot, err, val);
    }
}

static void suicide_bot( RW_Active_Robot *bot ) {
    bot->damage = 0;
}

static void write_reg( RW_Battle *b, RW_Active_Robot *bot, int reg, int value ) {
    switch(reg) {
        /* Do nothing for read-only registers */
        case reg_chronon:
        case reg_collision:
        case reg_damage:
        case reg_energy:
        case reg_friend:
        case reg_id:
        case reg_kills:
        case reg_radar:
        case reg_range:
        case reg_robots:
        case reg_teammates:
        case reg_wall:
        case reg_x:
        case reg_y:
            return;
        /* Values that need to be clipped between 0-359 */
        case reg_aim:
            if( value < 0 ) {
                bot->regs[reg] = 360 - ((value * -1) % 360);
            }
            else {
                bot->regs[reg] = value % 360;
            }
            return;
        default:
            bot->regs[reg] = value;
            return;
        /* TODO: History, channel, and signal */
        case reg_bullet:
            add_event(b, event_bullet, bot, value);
            return;
        case reg_fire:
            if( bot->robot->hardware.bullet == 2 ) {
                add_event(b, event_explosive, bot, value);
            }
            else {
                add_event(b, event_bullet, bot, value);
            }
            return;
        case reg_hellbore:
            if( bot->robot->hardware.hellbore ) {
                add_event(b, event_hellbore, bot, value);
            }
            return;
        case reg_mine:
            if( bot->robot->hardware.mine ) {
                add_event(b, event_mine, bot, value);
            }
            return;
        case reg_missile:
            if( bot->robot->hardware.missile ) {
                add_event(b, event_missile, bot, value);
            }
            return;
        case reg_movex:
            add_event(b, event_movex, bot, value);
            return;
        case reg_movey:
            add_event(b, event_movey, bot, value);
            return;
        case reg_nuke:
            if( bot->robot->hardware.tacnuke ) {
                add_event(b, event_nuke, bot, value);
            }
            return;
        case reg_shield:
            add_event(b, event_shield, bot, value);
            return;
        case reg_speedx:
            add_event(b, event_speedx, bot, value);
            return;
        case reg_speedy:
            add_event(b, event_speedy, bot, value);
            return;
        case reg_stunner:
            if( bot->robot->hardware.stunner ) {
                add_event(b, event_stunner, bot, value);
            }
            return;
    }
}

static int get_reg( RW_Battle *b, RW_Active_Robot *bot, int reg ) {
    int temp_range;
    int range;
    int angle, angle2;
    int turret;
    RW_Shot *shot;
    RW_Active_Robot *bot2;
    RW_Robot_Iter ri;
    RW_Shot_Iter si;
    switch(reg) {
        /* Return 0 for write-only registers */
        case reg_bullet:
        case reg_fire:
        case reg_mine:
        case reg_missile:
        case reg_movex:
        case reg_movey:
        case reg_nuke:
        case reg_stunner:
            return 0;
        default:
            return bot->regs[reg];
        /* TODO: History, probe, signal, teammates */
        case reg_chronon:
            return b->chronon;
        case reg_collision:
            RW_Reset_Robot_Iter(b, &ri, bot);
            while( (bot2 = RW_Robot_Next(&ri)) ) {
                range = square(bot->regs[reg_x] - bot2->regs[reg_x]) +
                    square(bot->regs[reg_y] - bot2->regs[reg_y]);
                if( range < (10 + 10) * (10 + 10) ) {
                    return 1;
                }
            }
            return 0;
        case reg_energy:
            return bot->energy;
        case reg_radar:
            /* For the radar register, we'll be dealing with the range in terms
               of range squared. This will limit us to at most 1 call to sqrt
               per radar check without affecting accuracy. */
            range = 600 * 600;
            turret = bot->regs[reg_aim] + bot->regs[reg_scan];
            if( turret < 0 ) {
                turret = 360 - ((turret * -1) % 360);
            }
            else {
                turret = turret % 360;
            }
            RW_Reset_Shot_Iter(b, &si);
            while( (shot = RW_Shot_Next(&si)) ) {
                angle = robo_atan2(shot->y - bot->regs[reg_y], shot->x - bot->regs[reg_x]);
                if( abs(turret - angle) <= 30 || abs(turret - angle) >= 330 ) {
                    temp_range = square(shot->x - bot->regs[reg_x]) +
                        square(shot->y - bot->regs[reg_y]);
                    range = temp_range < range ? temp_range : range;
                }
            }
            if( range != 600 * 600 ) {
                return sqrt(range);
            }
            else {
                return 0;
            }
        case reg_range:
            /* For the radar register we ignore the actual size of the shot
               because the radar works on set arc. Unfortunately, since we don't
               work with a set arc when dealing with range we need to consider
               the size of opposing robots. The trigonometry involved currently
               means 2 calls to atan2 and 1 call to sqrt per robot for every
               range read, which isn't good. */
            range = 600;
            turret = bot->regs[reg_aim] + bot->regs[reg_look];
            if( turret < 0 ) {
                turret = 360 - ((turret * -1) % 360);
            }
            else {
                turret = turret % 360;
            }
            RW_Reset_Robot_Iter(b, &ri, bot);
            while( (bot2 = RW_Robot_Next(&ri)) ) {
                angle = robo_atan2(bot2->regs[reg_y] - bot->regs[reg_y],
                    bot2->regs[reg_x] - bot->regs[reg_x]);
                temp_range = sqrt(square(bot2->regs[reg_y] - bot->regs[reg_y]) +
                    square(bot2->regs[reg_x] - bot->regs[reg_x]));
                angle2 = robo_atan2_raw(10, temp_range);
                if( abs(turret - angle) < angle2 || abs(turret - angle) > 360 - angle2 ) {
                    range = (temp_range < range && temp_range != 0) ? temp_range : range;
                }
            }
            if( range != 600 ) {
                return range;
            }
            else {
                return 0;
            }
        case reg_robots:
            return RW_Alive_Robots(b);
        case reg_wall:
            if( bot->regs[reg_x] < 10 || bot2->regs[reg_x] > 290 ||
                bot->regs[reg_y] < 10 || bot2->regs[reg_y] > 290 ) {
                return 1;
            }
            else {
                return 0;
            }
    }
}

static int push_val( RW_Battle *b, RW_Active_Robot *bot, int value ) {
    if( bot->stack_loc >= 49 ) {
        /* Stack overflow */
        report_error(b, bot, error_stack_of, 0);
        suicide_bot(bot);
        return -1;
    }
    bot->stack[bot->stack_loc+1] = value;
    bot->stack_loc++;
    return 0;
}

int RW_Run_Code( RW_Battle *b, RW_Active_Robot *bot ) {
    int opcode, reg1, reg2, reg3, imme;
    int reg1_val, reg2_val, reg3_val;
    /* Convenience macros */
#define putr(r,v) write_reg(b, bot, r, v)
#define getr(r) get_reg(b, bot, r)
    if( bot->code_loc >= bot->robot->code_size ) {
        /* Reached/exceeded EOF, die */
        report_error(b, bot, error_eof, bot->code_loc);
        suicide_bot(bot);
        return 1;
    }
    decode_op(bot->robot->code[bot->code_loc], opcode, reg1, reg2, reg3, imme);
    bot->code_loc++;
    /* The big switch statement */
    switch( opcode ) {
        case op_nop:
            break;
        case op_ne:
            putr(reg1, getr(reg2) != getr(reg3));
            break;
        case op_eq:
            putr(reg1, getr(reg2) == getr(reg3));
            break;
        case op_lt:
            putr(reg1, getr(reg2) < getr(reg3));
            break;
        case op_gt:
            putr(reg1, getr(reg2) > getr(reg3));
            break;
        case op_add:
            putr(reg1, getr(reg2) + getr(reg3));
            break;
        case op_sub:
            putr(reg1, getr(reg2) - getr(reg3));
            break;
        case op_mul:
            putr(reg1, getr(reg2) * getr(reg3));
            break;
        case op_div:
            putr(reg1, getr(reg2) / getr(reg3));
            break;
        case op_mod:
            putr(reg1, getr(reg2) % getr(reg3));
            break;
        case op_and:
            putr(reg1, getr(reg2) && getr(reg3));
            break;
        case op_or:
            putr(reg1, getr(reg2) || getr(reg3));
            break;
        case op_xor:
            /* C doesn't have a logical XOR, can you believe that? */
            putr(reg1, (!!getr(reg2)) ^ (!!getr(reg3)));
            break;
        case op_max:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, reg2_val > reg3_val ? reg2_val : reg3_val);
            break;
        case op_min:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, reg2_val < reg3_val ? reg2_val : reg3_val);
            break;
        case op_dist:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, sqrt((reg2_val*reg2_val)+(reg3_val*reg3_val)));
            break;
        case op_sin:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, robo_sin(reg2_val, reg3_val));
            break;
        case op_cos:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, robo_cos(reg2_val, reg3_val));
            break;
        case op_tan:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, robo_tan(reg2_val, reg3_val));
            break;
        case op_arcsin:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, robo_asin(reg2_val, reg3_val));
            break;
        case op_arccos:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, robo_acos(reg2_val, reg3_val));
            break;
        case op_arctan:
            reg2_val = getr(reg2);
            reg3_val = getr(reg3);
            putr(reg1, robo_atan2(reg2_val, reg3_val));
            break;
        case op_call:
            if( push_val(b, bot, bot->code_loc) ) {
                return 1;
            }
            bot->code_loc = imme;
            break;
        case op_jump:
            bot->code_loc = imme;
            break;
        case op_mova:
            putr(reg_a, sign_extend(imme));
            return RW_Run_Code(b, bot); /* Op doesn't use CPU time */
        case op_movb:
            putr(reg_b, sign_extend(imme));
            return RW_Run_Code(b, bot); /* Op doesn't use CPU time */
        case op_push:
            if( push_val(b, bot, sign_extend(imme)) ) {
                return 1;
            }
        default:
            /* Unknown opcode */
            report_error(b, bot, error_unknown_op, opcode);
            suicide_bot(bot);
            return 1;
        case op_two_reg:
            switch(reg3) {
                case op_mov:
                    putr(reg1, getr(reg2));
                    break;
                case op_not:
                    putr(reg1, !getr(reg2));
                    break;
                case op_chs:
                    putr(reg1, -1 * getr(reg2));
                    break;
                case op_abs:
                    putr(reg1, abs(getr(reg2)));
                    break;
                case op_sqrt:
                    putr(reg1, sqrt(getr(reg2)));
                    break;
                case op_vrecall:
                    reg2_val = getr(reg2);
                    if( reg2_val < 0 || reg2_val >= 100 ) {
                        /* Out of range */
                        report_error(b, bot, error_out_of_range, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    putr(reg1, bot->vector[reg2_val]);
                    break;
                case op_vstore:
                    reg1_val = getr(reg1);
                    if( reg1_val < 0 || reg1_val >= 100 ) {
                        /* Out of range */
                        report_error(b, bot, error_out_of_range, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    bot->vector[reg1_val] = getr(reg2);
                    break;
                case op_setparam:
                    /* TODO */
                    break;
            }
            break;
        case op_one_reg:
            switch(reg3) {
                case op_test:
                    if( !getr(reg1) ) {
                        bot->code_loc++;
                    }
                    return RW_Run_Code(b, bot); /* Op doesn't use CPU time */
                case op_icon:
                case op_sound:
                case op_roll:
                case op_random:
                case op_print:
                    /* TODO */
                    break;
                case op_peek:
                    if( bot->stack_loc < 0 ) {
                        /* Stack underflow */
                        report_error(b, bot, error_stack_uf, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    putr(reg1, bot->stack[bot->stack_loc]);
                    break;
                case op_pop:
                    if( bot->stack_loc < 0 ) {
                        /* Stack underflow */
                        report_error(b, bot, error_stack_uf, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    putr(reg1, bot->stack[bot->stack_loc]);
                    bot->stack_loc--;
                    break;
                case op_pushr:
                    push_val(b, bot, getr(reg1));
                    break;
                case op_debug:
                    report_error(b, bot, error_debug, getr(reg1));
                    break;
            }
            break;
        case op_zero_reg:
            /* Second level switch statement */
            switch(reg3) {
                case op_beep:
                    /* TODO */
                    break;
                case op_drop:
                    if( bot->stack_loc <= 0 ) {
                        /* Stack underflow */
                        report_error(b, bot, error_stack_uf, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    bot->stack_loc--;
                    break;
                case op_dropall:
                    bot->stack_loc = -1;
                    break;
                case op_dup:
                    if( bot->stack_loc >= 49 ) {
                        /* Stack overflow */
                        report_error(b, bot, error_stack_of, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    bot->stack[bot->stack_loc+1] = bot->stack[bot->stack_loc];
                    bot->stack_loc++;
                    break;
                case op_end:
                    suicide_bot(bot);
                    return 1;
                case op_recall:
                    /* TODO */
                    break;
                case op_return:
                    if( bot->stack_loc < 0 ) {
                        /* Stack underflow */
                        report_error(b, bot, error_stack_uf, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    bot->code_loc = bot->stack[bot->stack_loc];
                    bot->stack_loc--;
                    break;
                case op_store:
                    /* TODO */
                    break;
                case op_swap:
                    if( bot->stack_loc < 2 ) {
                        /* Stack underflow */
                        report_error(b, bot, error_stack_uf, 0);
                        suicide_bot(bot);
                        return 1;
                    }
                    reg1 = bot->stack[bot->stack_loc];
                    bot->stack[bot->stack_loc] = bot->stack[bot->stack_loc - 1];
                    bot->stack[bot->stack_loc - 1] = reg1;
                    break;
                case op_sync:
                    return 1;
            }
    }
    return 0;
    /* Clear the helper macros to avoid mucking up the namespace */
#undef putr
#undef getr
}
