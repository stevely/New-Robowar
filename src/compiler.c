/*
 * compiler.c
 *
 * By Steven Smith
 */

#include <stdio.h>
#include <stdlib.h>
#include "robocode.h"
#include "robocompiler.h"
#include "roboconfig.h"
#include "robotfile.h"

void write_out_bot( const char *fname, RW_Robo_Op *code, size_t code_size ) {
    RW_Robot_File_Entry *e = NULL;
    RW_Hardware_Spec hw;
    FILE *fp;
    char *n;
    int i,j;
    fp = fopen(fname, "wb");
    if( fp ) {
        /* Get name */
        n = RW_Config_Lookup("name");
        if( !n ) {
            for( i = 0; fname[i]; i++ ) ;
            n = (char*)malloc(sizeof(char) * (i+1));
            for( j = 0; j < i && fname[j] != '.'; j++ ) {
                n[j] = fname[j];
            }
            n[j] = 0;
        }
        /* Get hardware */
        hw = RW_Get_HW_From_Config();
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
    FILE *fp, *fp2;
    RW_Robo_Op *code;
    size_t code_size;
    int err_l;
    if( argc < 3 ) {
        fprintf(stdout, "Usage, %s output source [config]\n", argv[0]);
        return 0;
    }
    fp = fopen(argv[2], "r");
    if( argc > 3 ) {
        fp2 = fopen(argv[3], "r");
        if( !fp2 ) {
            fprintf(stdout, "Failed to open configuration file.\n");
        }
        err_l = RW_Read_Config_File_f(fp2);
        if( err_l ) {
            fprintf(stdout, "Error reading configuration file on line %d.\n", err_l);
        }
    }
    if( !fp ) {
        fprintf(stdout, "Failed to open source file.\n");
        return -1;
    }
    code = RW_Compile_Robot_f(fp, &code_size);
    if( code ) {
        write_out_bot(argv[1], code, code_size);
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
