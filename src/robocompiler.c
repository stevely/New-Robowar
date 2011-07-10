/*
 * robocompiler.c
 *
 * By Steven Smith
 */

#include "robocode.h"
#include <stdlib.h>
#include <stdio.h>

/*
 * Tokens generated by the lexer.
 */
enum token {
    token_ident, /* Generic identifier, gets replaced in the second pass of lexer */
    token_num,
    token_comma,
    token_colon,
    token_label,
    /* Instructions */
    token_abs,
    token_add,
    token_and,
    token_arccos,
    token_arcsin,
    token_arctan,
    token_beep,
    token_call,
    token_chs,
    token_cos,
    token_debug,
    token_dist,
    token_div,
    token_drop,
    token_dropall,
    token_dup,
    token_end,
    token_eq,
    token_gt,
    token_icon,
    token_if,
    token_ife,
    token_ifeg,
    token_ifg,
    token_jump,
    token_lt,
    token_max,
    token_min,
    token_mod,
    token_mov,
    token_mul,
    token_ne,
    token_nop,
    token_not,
    token_or,
    token_peek,
    token_pop,
    token_print,
    token_push,
    token_pushr,
    token_random,
    token_recall,
    token_return,
    token_roll,
    token_setparam,
    token_sin,
    token_sound,
    token_sqrt,
    token_store,
    token_sub,
    token_swap,
    token_sync,
    token_tan,
    token_test,
    token_vrecall,
    token_vstore,
    token_xor,
    token_reg /* Registers */
};

typedef struct token_list {
    enum token t;
    char *ident;
    int value;
    int line_number;
    struct token_list *next;
} token_list;

static token_list *tl = NULL;
static token_list *tl_end = NULL;

#define IDENTBUF_SIZE 50
static char ident_buf[IDENTBUF_SIZE + 1];

#define isalpha(c) \
    ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define isalphanum(c) \
    ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
#define isnum(c) \
    (c >= '0' && c <= '9')

static void build_token_ident( int pos, int current_line ) {
    char *s;
    int i;
    token_list *new_token = (token_list*)malloc(sizeof(token_list));
    s = (char*)malloc(sizeof(char) * pos + 1); /* I know this is 1 * pos + 1 */
    new_token->t = token_ident;
    for( i = 0; i < pos; i++ ) {
        s[i] = ident_buf[i];
    }
    s[pos] = '\0';
    new_token->ident = s;
    new_token->line_number = current_line;
    new_token->next = NULL;
    if( !tl ) {
        tl = new_token;
        tl_end = new_token;
    }
    else {
        tl_end->next = new_token;
        tl_end = new_token;
    }
}

static void build_token_num( int pos, int current_line ) {
    token_list *new_token = (token_list*)malloc(sizeof(token_list));
    new_token->t = token_num;
    new_token->ident = NULL;
    ident_buf[pos] = '\0';
    new_token->value = atoi(ident_buf);
    new_token->line_number = current_line;
    new_token->next = NULL;
    if( !tl ) {
        tl = new_token;
        tl_end = new_token;
    }
    else {
        tl_end->next = new_token;
        tl_end = new_token;
    }
}

static void build_token_comma( int current_line ) {
    token_list *new_token = (token_list*)malloc(sizeof(token_list));
    new_token->t = token_comma;
    new_token->ident = NULL;
    new_token->line_number = current_line;
    new_token->next = NULL;
    if( !tl ) {
        tl = new_token;
        tl_end = new_token;
    }
    else {
        tl_end->next = new_token;
        tl_end = new_token;
    }
}

static void build_token_colon( int current_line ) {
    token_list *new_token = (token_list*)malloc(sizeof(token_list));
    new_token->t = token_colon;
    new_token->ident = NULL;
    new_token->line_number = current_line;
    new_token->next = NULL;
    if( !tl ) {
        tl = new_token;
        tl_end = new_token;
    }
    else {
        tl_end->next = new_token;
        tl_end = new_token;
    }
}

static int build_token_list_f( FILE *fp ) {
    int curr_char;
    int ident_pos;
    int current_line;
    if( !fp ) {
        return -1;
    }
    curr_char = fgetc(fp);
    current_line = 1;
    while( curr_char != EOF ) {
        /* Ident builder */
        if( isalpha(curr_char) ) {
            ident_pos = 0;
            do {
                ident_buf[ident_pos] = curr_char;
                ident_pos++;
                curr_char = fgetc(fp);
            } while( curr_char != EOF && isalphanum(curr_char) && ident_pos < IDENTBUF_SIZE );
            build_token_ident(ident_pos, current_line);
            continue;
        }
        /* Number builder */
        else if( isnum(curr_char) || curr_char == '-' ) {
            ident_pos = 0;
            do {
                ident_buf[ident_pos] = curr_char;
                ident_pos++;
                curr_char = fgetc(fp);
            } while( curr_char != EOF && isnum(curr_char)  && ident_pos < IDENTBUF_SIZE );
            build_token_num(ident_pos, current_line);
            continue;
        }
        /* Grab commas */
        else if( curr_char == ',' ) {
            build_token_comma(current_line);
        }
        /* Grab semicolons */
        else if( curr_char == ':' ) {
            build_token_colon(current_line);
        }
        /* Handle comments */
        else if( curr_char == '#' ) {
            /* Eat everything until EOL */
            do {
                curr_char = fgetc(fp);
            } while( curr_char != EOF && curr_char != '\n' && curr_char != '\r' );
        }
        if( curr_char == '\n' ) {
            current_line++;
        }
        curr_char = fgetc(fp);
    }
    return 0;
}

static int ident_compare( char *c1, char *c2 ) {
    int i = 0;
    if( c1 && c2 ) {
        while( c1[i] && c2[i] && c1[i] == c2[i] ) {
            i++;
        }
        if( c1[i] == c2[i] ) {
            return 1;
        }
    }
    return 0;
}

static int token_convert( token_list *tok, char *c, enum token t ) {
    if( ident_compare(tok->ident, c) ) {
        tok->t = t;
        free(tok->ident);
        tok->ident = NULL;
        return 1;
    }
    else {
        return 0;
    }
}

static int reg_convert( token_list *tok, char *c, enum special_reg r ) {
    if( ident_compare(tok->ident, c) ) {
        tok->t = token_reg;
        tok->value = (int)r;
        free(tok->ident);
        tok->ident = NULL;
        return 1;
    }
    else {
        return 0;
    }
}

static int reg_check( token_list *tok ) {
    int value, i;
    char *c = tok->ident;
    if( c && c[0] == 'r' ) {
        i = 1;
        value = 0;
        while( c[i] && c[i] >= '0' && c[i] <= '9' ) {
            value = value * 10 + (c[i] - '0');
            i++;
        }
        if( !c[i] && value < 450 ) {
            tok->t = token_reg;
            tok->value = value;
            free(tok->ident);
            tok->ident = NULL;
            return 1;
        }
        else { /* Bad value or found something that's not a number */
            return 0;
        }
    }
    else {
        return 0;
    }
}

/*
 * Converts all the ident tokens we generated in the first pass of the lexer into
 * proper tokens. This will vastly simplify the parse phase.
 */
static void convert_ident_tokens() {
    token_list *tok = tl;
    while( tok != NULL ) {
        if( tok->t == token_ident ) {
            /* Instructions */
            if( token_convert(tok, "abs", token_abs) );
            else if( token_convert(tok, "add", token_add) );
            else if( token_convert(tok, "and", token_and) );
            else if( token_convert(tok, "arccos", token_arccos) );
            else if( token_convert(tok, "arcsin", token_arcsin) );
            else if( token_convert(tok, "arctan", token_arctan) );
            else if( token_convert(tok, "beep", token_beep) );
            else if( token_convert(tok, "call", token_call) );
            else if( token_convert(tok, "chs", token_chs) );
            else if( token_convert(tok, "cos", token_cos) );
            else if( token_convert(tok, "debug", token_debug) );
            else if( token_convert(tok, "dist", token_dist) );
            else if( token_convert(tok, "div", token_div) );
            else if( token_convert(tok, "drop", token_drop) );
            else if( token_convert(tok, "dropall", token_dropall) );
            else if( token_convert(tok, "dup", token_dup) );
            else if( token_convert(tok, "end", token_end) );
            else if( token_convert(tok, "eq", token_eq) );
            else if( token_convert(tok, "gt", token_gt) );
            else if( token_convert(tok, "icon", token_icon) );
            else if( token_convert(tok, "if", token_if) );
            else if( token_convert(tok, "ife", token_ife) );
            else if( token_convert(tok, "ifeg", token_ifeg) );
            else if( token_convert(tok, "ifg", token_ifg) );
            else if( token_convert(tok, "jump", token_jump) );
            else if( token_convert(tok, "lt", token_lt) );
            else if( token_convert(tok, "max", token_max) );
            else if( token_convert(tok, "min", token_min) );
            else if( token_convert(tok, "mod", token_mod) );
            else if( token_convert(tok, "mov", token_mov) );
            else if( token_convert(tok, "mul", token_mul) );
            else if( token_convert(tok, "ne", token_ne) );
            else if( token_convert(tok, "nop", token_nop) );
            else if( token_convert(tok, "not", token_not) );
            else if( token_convert(tok, "or", token_or) );
            else if( token_convert(tok, "peek", token_peek) );
            else if( token_convert(tok, "pop", token_pop) );
            else if( token_convert(tok, "print", token_print) );
            else if( token_convert(tok, "push", token_push) );
            else if( token_convert(tok, "random", token_random) );
            else if( token_convert(tok, "recall", token_recall) );
            else if( token_convert(tok, "return", token_return) );
            else if( token_convert(tok, "roll", token_roll) );
            else if( token_convert(tok, "setparam", token_setparam) );
            else if( token_convert(tok, "sin", token_sin) );
            else if( token_convert(tok, "sound", token_sound) );
            else if( token_convert(tok, "sqrt", token_sqrt) );
            else if( token_convert(tok, "store", token_store) );
            else if( token_convert(tok, "sub", token_sub) );
            else if( token_convert(tok, "swap", token_swap) );
            else if( token_convert(tok, "sync", token_sync) );
            else if( token_convert(tok, "tan", token_tan) );
            else if( token_convert(tok, "test", token_test) );
            else if( token_convert(tok, "vrecall", token_vrecall) );
            else if( token_convert(tok, "vstore", token_vstore) );
            else if( token_convert(tok, "xor", token_xor) );
            /* Registers */
            else if( reg_convert(tok, "aim", reg_aim) );
            else if( reg_convert(tok, "bullet", reg_bullet) );
            else if( reg_convert(tok, "channel", reg_channel) );
            else if( reg_convert(tok, "chronon", reg_chronon) );
            else if( reg_convert(tok, "collision", reg_collision) );
            else if( reg_convert(tok, "damage", reg_damage) );
            else if( reg_convert(tok, "energy", reg_energy) );
            else if( reg_convert(tok, "fire", reg_fire) );
            else if( reg_convert(tok, "friend", reg_friend) );
            else if( reg_convert(tok, "hellbore", reg_hellbore) );
            else if( reg_convert(tok, "history", reg_history) );
            else if( reg_convert(tok, "id", reg_id) );
            else if( reg_convert(tok, "kills", reg_kills) );
            else if( reg_convert(tok, "look", reg_look) );
            else if( reg_convert(tok, "mine", reg_mine) );
            else if( reg_convert(tok, "missile", reg_missile) );
            else if( reg_convert(tok, "movex", reg_movex) );
            else if( reg_convert(tok, "movey", reg_movey) );
            else if( reg_convert(tok, "nuke", reg_nuke) );
            else if( reg_convert(tok, "probe", reg_probe) );
            else if( reg_convert(tok, "radar", reg_radar) );
            else if( reg_convert(tok, "range", reg_range) );
            else if( reg_convert(tok, "robots", reg_robots) );
            else if( reg_convert(tok, "scan", reg_scan) );
            else if( reg_convert(tok, "shield", reg_shield) );
            else if( reg_convert(tok, "signal", reg_signal) );
            else if( reg_convert(tok, "speedx", reg_speedx) );
            else if( reg_convert(tok, "speedy", reg_speedy) );
            else if( reg_convert(tok, "stunner", reg_stunner) );
            else if( reg_convert(tok, "teammates", reg_teammates) );
            else if( reg_convert(tok, "wall", reg_wall) );
            else if( reg_convert(tok, "x", reg_x) );
            else if( reg_convert(tok, "y", reg_y) );
            else if( reg_check(tok) );
            else { /* Default to label */
                tok->t = token_label;
            }
        }
        tok = tok->next;
    }
}

enum errors {
    err_no_err = 0,
    err_reg1_expected = 1,
    err_comma_expected = 2,
    err_unexpected_eof = 3,
    err_regnum2_expected = 4,
    err_regnum3_expected = 5,
    err_labelnum_expected = 6,
    err_duplabel = 7,
    err_colon_expected = 8,
    err_labelnumreg_expected = 9,
    err_unknown_token = 10,
    err_nonexistant_label = 11
};

static int error = 0;
static int error_line = 0;

typedef struct op_list {
    RW_Robo_Op ops[5000]; /* We'll start with 5k, if we need more we'll alloc another 5k */
    int length;
    struct op_list *next;
} op_list;

typedef struct label_loc_map {
    char *name;
    int location;
    struct label_loc_map *prev;
} label_loc_map;

static op_list *ops = NULL;

static label_loc_map *lmap = NULL; /* Label->location map */
static label_loc_map *fmap = NULL; /* Ops that are waiting for a label to be resolved */

static op_list * new_op_list() {
    op_list *new_list;
    new_list = (op_list*)malloc(sizeof(op_list));
    new_list->length = 0;
    new_list->next = NULL;
    return new_list;
}

static void create_instruction( RW_Robo_Op instruction ) {
    op_list *list;
    if( !ops ) {
        ops = new_op_list();
    }
    list = ops;
    while( list->next ) {
        list = list->next; /* Get last op list to append */
    }
    if( list->length == 5000 ) { /* Create new list if current list is full */
        list->next = new_op_list();
        list = list->next;
    }
    list->ops[list->length] = instruction;
    list->length++;
}

static void update_branch_instruction( label_loc_map *map, int location ) {
    op_list *list;
    int op_loc;
    label_loc_map *other_map;
    list = ops;
    op_loc = map->location;
    while( op_loc > 5000 ) {
        op_loc -= 5000;
        list = list->next;
    }
    update_op_imme(list->ops[op_loc], location);
    /* Remove the branch from the update list */
    if( map == fmap ) {
        fmap = fmap->prev;
    }
    else {
        /* Find map entry right before this one */
        other_map = fmap;
        while( other_map->prev != map ) {
            other_map = other_map->prev;
        }
        other_map->prev = map->prev;
    }
    free(map);
}

static void create_mova( int value ) {
    RW_Robo_Op new_op;
    encode_op_imme(new_op, op_mova, value);
    create_instruction(new_op);
}

static void create_movb( int value ) {
    RW_Robo_Op new_op;
    encode_op_imme(new_op, op_movb, value);
    create_instruction(new_op);
}

static token_list * three_reg_op( token_list *t, int op ) {
    int reg1, reg2, reg3;
    RW_Robo_Op new_op;
    token_list *tok = t;
    /* Get reg1 */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_reg ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_reg1_expected;
        return NULL;
    }
    reg1 = tok->value;
    /* Eat a comma, get reg2 or first immediate */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_comma ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_comma_expected;
        return NULL;
    }
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t == token_reg ) {
        reg2 = tok->value;
    }
    else if( tok->t == token_num ) {
        reg2 = reg_a;
        create_mova(tok->value);
    }
    else {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_regnum2_expected;
        return NULL;
    }
    /* Eat a comma, get reg3 or second immediate */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_comma ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_comma_expected;
        return NULL;
    }
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t == token_reg ) {
        reg3 = tok->value;
    }
    else if( tok->t == token_num ) {
        reg3 = reg_b;
        create_movb(tok->value);
    }
    else {
        error_line = tok->line_number;
        error = err_regnum3_expected;
        return NULL;
    }
    /* Create instruction */
    encode_op_regs(new_op, op, reg1, reg2, reg3);
    create_instruction(new_op);
    return tok->next;
}

static token_list * two_reg_op( token_list *t, int op ) {
    int reg1, reg2;
    RW_Robo_Op new_op;
    token_list *tok = t;
    /* Get reg1 */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_reg ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_reg1_expected;
        return NULL;
    }
    reg1 = tok->value;
    /* Eat a comma, get reg2 or first immediate */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_comma ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_comma_expected;
        return NULL;
    }
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t == token_reg ) {
        reg2 = tok->value;
    }
    else if( tok->t == token_num ) {
        reg2 = reg_a;
        create_mova(tok->value);
    }
    else {
        error_line = tok->line_number;
        error = err_regnum2_expected;
        return NULL;
    }
    /* Create instruction */
    encode_op_regs(new_op, op_two_reg, reg1, reg2, op);
    create_instruction(new_op);
    return tok->next;
}

static token_list * one_reg_op( token_list *t, int op ) {
    int reg1;
    RW_Robo_Op new_op;
    token_list *tok = t;
    /* Get reg1 */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_reg ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_reg1_expected;
        return NULL;
    }
    reg1 = tok->value;
    /* Create instruction */
    encode_op_regs(new_op, op_one_reg, reg1, 0, op);
    create_instruction(new_op);
    return tok->next;
}

static token_list * zero_reg_op( token_list *t, int op ) {
    RW_Robo_Op new_op;
    /* Create instruction */
    encode_op_regs(new_op, op_zero_reg, 0, 0, op);
    create_instruction(new_op);
    return t->next;
}

static int check_label( token_list *t ) {
    label_loc_map *map;
    op_list *list;
    int loc;
    /* Check the label->location map */
    map = lmap;
    while( map && !ident_compare(map->name, t->ident) ) {
        map = map->prev;
    }
    if( map ) { /* Found a match */
        return map->location;
    }
    else { /* No match, add to waiting list */
        map = (label_loc_map*)malloc(sizeof(label_loc_map));
        map->name = t->ident;
        map->prev = fmap;
        loc = 0;
        list = ops;
        while( list->next ) {
            list = list->next;
            loc += 5000;
        }
        loc += list->length;
        map->location = loc;
        fmap = map;
        return 0;
    }
}

static token_list * branch_op( token_list *t ) {
    int reg, dest;
    RW_Robo_Op new_op;
    token_list *tok = t;
    int type = tok->t;
    /* Check if we need a test instruction */
    if( tok->t != token_call && tok->t != token_jump ) {
        /* Get reg1 */
        tok = tok->next;
        if( !tok ) {
            error = err_unexpected_eof;
            return NULL;
        }
        else if( tok->t != token_reg ) {
            /* Syntax error */
            error_line = tok->line_number;
            error = err_reg1_expected;
            return NULL;
        }
        reg = tok->value;
        /* Create test instruction */
        encode_op_regs(new_op, op_one_reg, reg, 0, op_test);
        create_instruction(new_op);
        /* Eat comma */
        tok = tok->next;
        if( !tok ) {
            error = err_unexpected_eof;
            return NULL;
        }
        else if( tok->t != token_comma ) {
            /* Syntax error */
            error_line = tok->line_number;
            error = err_comma_expected;
            return NULL;
        }
    }
    /* Every branch instruction has at least 1 dest */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_label && tok->t != token_num ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_labelnum_expected;
        return NULL;
    }
    if( tok->t == token_label ) {
        dest = check_label(tok);
    }
    else { /* tok->t == token_num */
        dest = tok->value;
    }
    /* Create first (and possibly last) branch instruction */
    if( type == token_jump || type == token_ifg || type == token_ifeg ) {
        encode_op_imme(new_op, op_jump, dest);
        create_instruction(new_op);
    }
    else {
        encode_op_imme(new_op, op_call, dest);
        create_instruction(new_op);
    }
    /* Do we have a second branch instruction we need to make? */
    if( type == token_ife || type == token_ifeg ) {
        /* Eat comma */
        tok = tok->next;
        if( !tok ) {
            error = err_unexpected_eof;
            return NULL;
        }
        else if( tok->t != token_comma ) {
            /* Syntax error */
            error_line = tok->line_number;
            error = err_comma_expected;
            return NULL;
        }
        tok = tok->next;
        if( !tok ) {
            error = err_unexpected_eof;
            return NULL;
        }
        else if( tok->t != token_label && tok->t != token_num ) {
            /* Syntax error */
            error_line = tok->line_number;
            error = err_labelnum_expected;
            return NULL;
        }
        if( tok->t == token_label ) {
            dest = check_label(tok);
        }
        else { /* tok->t == token_num */
            dest = tok->value;
        }
        /* Create second branch instruction */
        if( type == token_ifeg ) {
            encode_op_imme(new_op, op_jump, dest);
            create_instruction(new_op);
        }
        else {
            encode_op_imme(new_op, op_call, dest);
            create_instruction(new_op);
        }
    }
    return tok->next;
}

static token_list * push_op( token_list *t ) {
    token_list *tok = t;
    int dest, reg;
    RW_Robo_Op new_op;
    /* Push can take a register, immediate, or label.
       This makes things a bit hairy, as we don't have an instruction for both
       immediates and registers. Therefor, we have two different instructions,
       and pick which one depending on what we got. */
    tok = t->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t == token_reg ) {
        reg = tok->value;
        encode_op_regs(new_op, op_one_reg, reg, 0, op_pushr);
    }
    else if( tok->t == token_num ) {
        dest = tok->value;
        encode_op_imme(new_op, op_push, dest);
    }
    else if( tok->t == token_label ) {
        dest = check_label(tok);
        encode_op_imme(new_op, op_push, dest);
    }
    else {
        error_line = tok->line_number;
        error = err_labelnumreg_expected;
        return NULL;
    }
    create_instruction(new_op);
    return tok->next;
}

static token_list * create_label( token_list *t ) {
    int loc;
    token_list *tok = t;
    label_loc_map *new_entry, *map;
    op_list *list = ops;
    /* Check for duplicate label entry */
    map = lmap;
    while( map ) {
        if( ident_compare(map->name, t->ident) ) {
            /* Duplicate entry, error */
            error_line = t->line_number;
            error = err_duplabel;
            return NULL;
        }
        map = map->prev;
    }
    /* Create label entry */
    new_entry = (label_loc_map*)malloc(sizeof(label_loc_map));
    new_entry->name = tok->ident;
    loc = 0;
    while( list->next ) {
        list = list->next;
        loc += 5000;
    }
    new_entry->location = loc + list->length;
    new_entry->prev = lmap;
    lmap = new_entry;
    /* Correct any instructions that are waiting for this label */
    map = fmap;
    while( map ) {
        if( ident_compare(map->name, new_entry->name) ) {
            /* Found a match, update with correct location */
            update_branch_instruction(map, new_entry->location);
        }
        map = map->prev;
    }
    /* Eat a colon */
    tok = tok->next;
    if( !tok ) {
        error = err_unexpected_eof;
        return NULL;
    }
    else if( tok->t != token_colon ) {
        /* Syntax error */
        error_line = tok->line_number;
        error = err_colon_expected;
        return NULL;
    }
    return tok->next;
}

static int parse_token_list() {
    token_list *tok;
    tok = tl;
    ops = new_op_list();
    while( tok != NULL ) {
        switch( tok->t ) {
            /* 3-register instructions */
            case token_add:
                tok = three_reg_op(tok, op_add); break;
            case token_and:
                tok = three_reg_op(tok, op_and); break;
            case token_arccos:
                tok = three_reg_op(tok, op_arccos); break;
            case token_arcsin:
                tok = three_reg_op(tok, op_arcsin); break;
            case token_arctan:
                tok = three_reg_op(tok, op_arctan); break;
            case token_cos:
                tok = three_reg_op(tok, op_cos); break;
            case token_dist:
                tok = three_reg_op(tok, op_dist); break;
            case token_div:
                tok = three_reg_op(tok, op_div); break;
            case token_eq:
                tok = three_reg_op(tok, op_eq); break;
            case token_gt:
                tok = three_reg_op(tok, op_gt); break;
            case token_lt:
                tok = three_reg_op(tok, op_lt); break;
            case token_max:
                tok = three_reg_op(tok, op_max); break;
            case token_min:
                tok = three_reg_op(tok, op_min); break;
            case token_mod:
                tok = three_reg_op(tok, op_mod); break;
            case token_mul:
                tok = three_reg_op(tok, op_mul); break;
            case token_ne:
                tok = three_reg_op(tok, op_ne); break;
            case token_or:
                tok = three_reg_op(tok, op_or); break;
            case token_sin:
                tok = three_reg_op(tok, op_sin); break;
            case token_sub:
                tok = three_reg_op(tok, op_sub); break;
            case token_tan:
                tok = three_reg_op(tok, op_tan); break;
            case token_xor:
                tok = three_reg_op(tok, op_xor); break;
            /* 2-register instructions */
            case token_abs:
                tok = two_reg_op(tok, op_abs); break;
            case token_chs:
                tok = two_reg_op(tok, op_chs); break;
            case token_mov:
                tok = two_reg_op(tok, op_mov); break;
            case token_not:
                tok = two_reg_op(tok, op_not); break;
            case token_setparam:
                tok = two_reg_op(tok, op_setparam); break;
            case token_sqrt:
                tok = two_reg_op(tok, op_sqrt); break;
            case token_vrecall:
                tok = two_reg_op(tok, op_vrecall); break;
            case token_vstore:
                tok = two_reg_op(tok, op_vstore); break;
            /* 1-register instructions */
            case token_icon:
                tok = one_reg_op(tok, op_icon); break;
            case token_peek:
                tok = one_reg_op(tok, op_peek); break;
            case token_pop:
                tok = one_reg_op(tok, op_pop); break;
            case token_print:
                tok = one_reg_op(tok, op_print); break;
            case token_random:
                tok = one_reg_op(tok, op_random); break;
            case token_roll:
                tok = one_reg_op(tok, op_roll); break;
            case token_sound:
                tok = one_reg_op(tok, op_sound); break;
            case token_test:
                tok = one_reg_op(tok, op_test); break;
            case token_debug:
                tok = one_reg_op(tok, op_debug); break;
            /* 0-register instructions */
            case token_beep:
                tok = zero_reg_op(tok, op_beep); break;
            case token_drop:
                tok = zero_reg_op(tok, op_drop); break;
            case token_dropall:
                tok = zero_reg_op(tok, op_dropall); break;
            case token_dup:
                tok = zero_reg_op(tok, op_dup); break;
            case token_end:
                tok = zero_reg_op(tok, op_end); break;
            case token_recall:
                tok = zero_reg_op(tok, op_recall); break;
            case token_return:
                tok = zero_reg_op(tok, op_return); break;
            case token_store:
                tok = zero_reg_op(tok, op_store); break;
            case token_swap:
                tok = zero_reg_op(tok, op_swap); break;
            case token_sync:
                tok = zero_reg_op(tok, op_sync); break;
            case token_nop:
                tok = zero_reg_op(tok, op_nop); break;
            /* Branching instructions */
            case token_if:
            case token_ife:
            case token_ifg:
            case token_ifeg:
            case token_jump:
            case token_call:
                tok = branch_op(tok); break;
            /* Push instruction */
            case token_push:
                tok = push_op(tok); break;
            /* Labels */
            case token_label:
                tok = create_label(tok); break;
            /* Default: Syntax error */
            default:
                /* TODO: error and die */
                error_line = tok->line_number;
                error = err_unknown_token;
                return error;
        }
    }
    return error;
}

static void compiler_cleanup() {
    token_list *tokens, *t;
    label_loc_map *map, *m;
    op_list *ol, *o;
    /* Eat token list */
    tokens = tl;
    while( tokens ) {
        t = tokens;
        tokens = tokens->next;
        if( t->ident ) {
            free(t->ident);
        }
        free(t);
    }
    tl_end = NULL;
    /* Note: The label/loc maps use shallow copies of the ident fields from the
       token list for the name fields. These strings are freed when the token
       list is freed, so we don't have to deal with them here. */
    /* Eat label map */
    map = lmap;
    while( map ) {
        m = map;
        map = map->prev;
        free(m);
    }
    /* Eat branch map */
    map = fmap;
    while( map ) {
        m = map;
        map = map->prev;
        free(m);
    }
    /* Eat tentative instruction list */
    ol = ops;
    while( ol ) {
        o = ol;
        ol = ol->next;
        free(o);
    }
}

const char * RW_Get_Compiler_Error( int *line_number ) {
    *line_number = error_line;
    switch( error ) {
        case err_no_err:
            return NULL;
        case err_reg1_expected:
            return "Register expected";
        case err_comma_expected:
            return "Comma expected";
        case err_unexpected_eof:
            *line_number = -1; /* No line number for unexpected EOF */
            return "Unexpected end of file";
        case err_regnum2_expected:
            return "Register or number expected";
        case err_regnum3_expected:
            return "Register or number expected";
        case err_labelnum_expected:
            return "Label or number expected";
        case err_duplabel:
            return "Duplicate label";
        case err_colon_expected:
            return "Colon expected";
        case err_labelnumreg_expected:
            return "Label, number, or register expected";
        case err_unknown_token:
            return "Unknown token";
        case err_nonexistant_label:
            return "Non-existant label";
        default:
            return "Unknown error";
    }
}

RW_Robo_Op * RW_Compile_Robot_f( FILE *fp, size_t *length ) {
    int l, i;
    op_list *ol;
    RW_Robo_Op *code = NULL;
    if( build_token_list_f(fp) ) {
        /* Error */
        goto cleanup;
    }
    convert_ident_tokens(); /* This phase can't fail, so no error checking */
    if( parse_token_list() ) {
        /* Error */
        goto cleanup;
    }
    if( fmap ) {
        error = err_nonexistant_label;
        goto cleanup;
    }
    /* Convert tentative instruction list into simple array */
    /* Step 1: Find the total length and allocate memory */
    l = 0;
    ol = ops;
    while( ol ) {
        if( ol->next ) {
            l += 5000;
        }
        else {
            l += ol->length;
        }
        ol = ol->next;
    }
    *length = l;
    code = (RW_Robo_Op*)malloc(sizeof(RW_Robo_Op) * l);
    /* Step 2: Copy over the ops */
    l = 0;
    ol = ops;
    while( ol ) {
        for( i = 0; i < ol->length; i++ ) {
            code[l] = ol->ops[i];
            l++;
        }
        ol = ol->next;
    }
    /* Step 3: Cleanup */
    cleanup:
    compiler_cleanup();
    return code;
}
