/*
 * compiletest.c
 *
 * By Steven Smith
 */

#include <stdlib.h>
#include <stdio.h>
#include "robocode.h"
#include "robotfile.h"
#include "robocompiler.h"

void print_code_3reg( int opcode, int reg1, int reg2, int reg3 ) {
    switch( opcode ) {
        case op_nop:
            printf("nop\n"); break;
        case op_ne:
            printf("ne %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_eq:
            printf("eq %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_lt:
            printf("lt %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_gt:
            printf("gt %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_add:
            printf("add %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_sub:
            printf("sub %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_mul:
            printf("mul %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_div:
            printf("div %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_mod:
            printf("mod %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_and:
            printf("and %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_or:
            printf("or %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_xor:
            printf("xor %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_max:
            printf("max %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_min:
            printf("min %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_dist:
            printf("dist %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_sin:
            printf("sin %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_cos:
            printf("cos %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_tan:
            printf("tan %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_arcsin:
            printf("arcsin %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_arccos:
            printf("arccos %d, %d, %d\n", reg1, reg2, reg3); break;
        case op_arctan:
            printf("arctan %d, %d, %d\n", reg1, reg2, reg3); break;
        default:
            break;
    }
}

void print_code_2reg( int opcode, int reg1, int reg2 ) {
    switch( opcode ) {
        case op_mov:
            printf("mov %d, %d\n", reg1, reg2); break;
        case op_not:
            printf("not %d, %d\n", reg1, reg2); break;
        case op_chs:
            printf("chs %d, %d\n", reg1, reg2); break;
        case op_abs:
            printf("abs %d, %d\n", reg1, reg2); break;
        case op_sqrt:
            printf("sqrt %d, %d\n", reg1, reg2); break;
        case op_vrecall:
            printf("vrecall %d, %d\n", reg1, reg2); break;
        case op_vstore:
            printf("vstore %d, %d\n", reg1, reg2); break;
        case op_setparam:
            printf("setparam %d, %d\n", reg1, reg2); break;
        default:
            break;
    }
}

void print_code_1reg( int opcode, int reg1 ) {
    switch( opcode ) {
        case op_test:
            printf("test %d\n", reg1); break;
        case op_icon:
            printf("icon %d\n", reg1); break;
        case op_sound:
            printf("sound %d\n", reg1); break;
        case op_roll:
            printf("roll %d\n", reg1); break;
        case op_random:
            printf("random %d\n", reg1); break;
        case op_print:
            printf("print %d\n", reg1); break;
        case op_debug:
            printf("debug %d\n", reg1); break;
        default:
            break;
    }
}

void print_code_0reg( int opcode ) {
    switch( opcode ) {
        case op_beep:
            printf("beep\n"); break;
        case op_drop:
            printf("drop\n"); break;
        case op_dropall:
            printf("dropall\n"); break;
        case op_dup:
            printf("dup\n"); break;
        case op_end:
            printf("end\n"); break;
        case op_recall:
            printf("recall\n"); break;
        case op_return:
            printf("return\n"); break;
        case op_store:
            printf("store\n"); break;
        case op_swap:
            printf("swap\n"); break;
        case op_sync:
            printf("sync\n"); break;
        default:
            break;
    }
}

void print_code_immed( int opcode, int imme ) {
    int se_imme = (sign_extend(imme));
    switch( opcode ) {
        case op_call:
            printf("call %d\n", se_imme); break;
        case op_jump:
            printf("jump %d\n", se_imme); break;
        case op_mova:
            printf("mova %d\n", se_imme); break;
        case op_movb:
            printf("movb %d\n", se_imme); break;
        case op_push:
            printf("push %d\n", se_imme); break;
        default:
            break;
    }
}

void decode_and_print_code( RW_Robo_Op *code, size_t length ) {
    size_t i;
    int opcode, reg1, reg2, reg3, imme;
    printf("Generated code:\n");
    for( i = 0; i < length; i++ ) {
        printf("%d: ", (int)i);
        decode_op(code[i], opcode, reg1, reg2, reg3, imme);
        switch( opcode ) {
            case op_nop:
            case op_ne:
            case op_eq:
            case op_lt:
            case op_gt:
            case op_add:
            case op_sub:
            case op_mul:
            case op_div:
            case op_mod:
            case op_and:
            case op_or:
            case op_xor:
            case op_max:
            case op_min:
            case op_dist:
            case op_sin:
            case op_cos:
            case op_tan:
            case op_arcsin:
            case op_arccos:
            case op_arctan:
                print_code_3reg(opcode, reg1, reg2, reg3);
                break;
            case op_two_reg:
                print_code_2reg(reg3, reg1, reg2);
                break;
            case op_one_reg:
                print_code_1reg(reg3, reg1);
                break;
            case op_zero_reg:
                print_code_0reg(reg3);
                break;
            case op_call:
            case op_jump:
            case op_mova:
            case op_movb:
            case op_push:
                print_code_immed(opcode, imme);
                break;
            default:
                printf("BAD OPERATOR\n");
                break;
        }
    }
}

void test_instr_encoding() {
    RW_Robo_Op op;
    int opcode, reg1, reg2, reg3, imme;
    encode_op_regs(op, op_add, 1, 2, 3);
    decode_op(op, opcode, reg1, reg2, reg3, imme);
    if( opcode != op_add ) {
        printf("opcode encoding/decoding error\n");
    }
    if( reg1 != 1 ) {
        printf("reg1 encoding/decoding error\n");
    }
    if( reg2 != 2 ) {
        printf("reg2 encoding/decoding error\n");
    }
    if( reg3 != 3 ) {
        printf("reg3 encoding/decoding error\n");
    }
}

int main( int argc, char **argv ) {
    FILE *fp;
    RW_Robo_Op *code;
    RW_Robot_File *rf;
    size_t code_size;
    int err_l;
    test_instr_encoding();
    if( argc != 2 ) {
        fprintf(stdout, "Usage, %s file\n", argv[0]);
        return 0;
    }
    rf = RW_Open_Robot(argv[1]);
    if( !rf ) {
        fprintf(stdout, "Failed to open robot file!\n");
        return 0;
    }
    code = RW_Get_Resource(rf, RW_CODE_ENTRY, &code_size);
    if( code ) {
        fprintf(stdout, "Robot successfully compiled!\n");
        decode_and_print_code(code, code_size);
    }
    else {
        fprintf(stdout, "Error reading code!\n");
    }
    return 0;
}
