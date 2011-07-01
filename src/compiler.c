/*
 * compiler.c
 *
 * By Steven Smith
 */

#include <stdio.h>
#include "robocode.h"
#include "robocompiler.h"
#include "robotfile.h"

void write_out_bot( const char *fname, RW_Robo_Op *code, size_t code_size ) {
    FILE *fp;
    RW_Robo_Op *c;
    RW_Robot_File output;
    fp = fopen(fname, "wb");
    if( fp ) {
        c = code;
        /* Setup file */
        output.magic = magic_field;
        output.version = version_number;
        /* Hard-code hardware for now */
        output.hardware.energy = 2;
        output.hardware.damage = 2;
        output.hardware.shield = 2;
        output.hardware.bullet = 2;
        output.hardware.probes = 1;
        output.hardware.negenergy = 1;
        output.hardware.hellbore = 1;
        output.hardware.mine = 1;
        output.hardware.missile = 1;
        output.hardware.tacnuke = 1;
        output.hardware.stunner = 1;
        /* Setup file entry */
        fwrite(&output, sizeof(RW_Robot_File), 1, fp);
        fwrite(&code_size, sizeof(unsigned int), 1, fp);
        fwrite("robocode", sizeof(char), 9, fp);
        fwrite(code, sizeof(RW_Robo_Op), code_size, fp);
        fclose(fp);
    }
}

int main( int argc, char **argv ) {
    FILE *fp;
    RW_Robo_Op *code;
    size_t code_size;
    int err_l;
    if( argc != 3 ) {
        fprintf(stdout, "Usage, %s source output\n", argv[0]);
        return 0;
    }
    fp = fopen(argv[1], "r");
    if( !fp ) {
        fprintf(stdout, "Failed to open source file.\n");
        return -1;
    }
    code = RW_Compile_Robot_f(fp, &code_size);
    if( code ) {
        write_out_bot(argv[2], code, code_size);
    }
    else {
        fprintf(stdout, "Error: %s", RW_Get_Compiler_Error(&err_l));
        if( err_l != -1 ) {
            fprintf(stdout, ". Line number: %d\n", err_l);
        }
        else {
            fprintf(stdout, ".\n");
        }
    }
    return 0;
}
