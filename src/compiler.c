/*
 * compiler.c
 *
 * By Steven Smith
 */

#include <stdio.h>
#include <stdlib.h>
#include "robocode.h"
#include "robocompiler.h"
#include "robotfile.h"

void write_out_bot( const char *fname, RW_Robo_Op *code, size_t code_size ) {
    RW_Robot_File_Entry *e = NULL;
    RW_Hardware_Spec hw;
    FILE *fp;
    char *n;
    int i,j;
    fp = fopen(fname, "wb");
    if( fp ) {
        for( i = 0; fname[i]; i++ ) ;
        n = (char*)malloc(sizeof(char) * (i+1));
        for( j = 0; j < i && fname[j] != '.'; j++ ) {
            n[j] = fname[j];
        }
        n[j] = 0;
        /* Hard-code hardware for now */
        hw.energy = 2;
        hw.damage = 2;
        hw.shield = 2;
        hw.bullet = 2;
        hw.probes = 1;
        hw.negenergy = 1;
        hw.hellbore = 1;
        hw.mine = 1;
        hw.missile = 1;
        hw.tacnuke = 1;
        hw.stunner = 1;
        /* Create entry for code */
        e = RW_Create_Robot_File_Entry(e, RW_CODE_ENTRY, code_size, sizeof(RW_Robo_Op), code);
        /* Write out robot file */
        RW_Write_Robot_File(fp, n, hw, e);
        /* Cleanup */
        RW_Free_Robot_File_Entry(e);
        free(n);
        free(code);
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
