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

int check_for_robot( FILE *fp ) {
    char m[4];
    if( fp == NULL ) {
        return 0;
    }
    else {
        fread(m, sizeof(char), 4, fp);
        fseek(fp, 0, SEEK_SET);
        if( RW_Check_Magic(m) ) {
            return 1;
        }
        else {
            return 0;
        }
    }
}

void insert_extra_file( FILE *fp, char *name, RW_Robot_File_Entry *entry ) {
    size_t size;
    int read;
    void *data;
    if( fp && name && entry ) {
        /* Step 1: Get size of data */
        fseek(fp, 0, SEEK_END);
        size = (size_t)ftell(fp);
        /* Step 2: Malloc the buffer */
        data = malloc(size);
        if( data == NULL ) {
            return;
        }
        /* Step 3: Grab data */
        fseek(fp, 0, SEEK_SET);
        read = fread(data, 1, size, fp);
        if( read != (int)size ) {
            free(data);
            return;
        }
        /* Step 4: Insert it into our index */
        RW_Create_Robot_File_Entry(entry, name, size, sizeof(void), data);
    }
}

void write_out_bot_f( const char *fname, RW_Robo_Op *code, size_t code_size,
    RW_Config_File *cf, FILE *source_fp, FILE *config_fp ) {
    RW_Robot_File_Entry *e = NULL;
    RW_Hardware_Spec hw;
    FILE *fp;
    char *n;
    int i,j;
    fp = fopen(fname, "wb");
    if( fp ) {
        /* Get name */
        n = RW_Config_Lookup(cf, "name");
        if( !n ) {
            for( i = 0; fname[i]; i++ ) ;
            n = (char*)malloc(sizeof(char) * (i+1));
            for( j = 0; j < i && fname[j] != '.'; j++ ) {
                n[j] = fname[j];
            }
            n[j] = 0;
        }
        /* Get hardware */
        hw = RW_Get_HW_From_Config(cf);
        /* Create entry for code */
        e = RW_Create_Robot_File_Entry(e, RW_CODE_ENTRY, code_size, sizeof(RW_Robo_Op), code);
        /* Get code source and config source */
        insert_extra_file(source_fp, RW_CODE_SOURCE, e);
        insert_extra_file(config_fp, RW_CONFIG_ENTRY, e);
        /* Write out robot file */
        RW_Write_Robot_File(fp, n, hw, e);
        /* Cleanup */
        RW_Free_Robot_File_Entry(e);
        free(n);
        free(code);
        fclose(fp);
    }
}

void write_out_bot_r( const char *fname, RW_Robot_File *rf, RW_Robo_Op *code,
    size_t code_size ) {
    RW_Robot_File_Entry *e = NULL;
    RW_Hardware_Spec hw;
    RW_Config_File config, *cf;
    char *s, *name;
    size_t i;
    FILE *fp;
    int err;
    cf = &config;
    if( rf == NULL ) {
        return;
    }
    fp = fopen(fname, "wb");
    if( fp == NULL ) {
        fprintf(stdout, "Failed opening destination file.\n");
        return;
    }
    /* See if we have a config file */
    s = RW_Get_Resource(rf, RW_CONFIG_ENTRY, NULL, &i);
    if( s ) {
        err = RW_Read_Config_File_s(s, i, cf);
        if( err ) {
            fprintf(stdout, "Error reading robot config file, line %d.\n", err);
            return;
        }
        /* Get name */
        name = RW_Config_Lookup(cf, "name");
        if( name == NULL ) {
            name = RW_Get_Robot_Name_From_File(rf);
        }
        /* Get hardware */
        hw = RW_Get_HW_From_Config(cf);
    }
    else {
        name = RW_Get_Robot_Name_From_File(rf);
        hw = RW_Get_Hardware_From_File(rf);
    }
    /* Create entry for code */
    e = RW_Create_Robot_File_Entry(e, RW_CODE_ENTRY, code_size, sizeof(RW_Robo_Op), code);
    /* Get code source and config source */
    s = RW_Get_Resource(rf, RW_CODE_SOURCE, NULL, &i);
    RW_Create_Robot_File_Entry(e, RW_CODE_SOURCE, i, sizeof(char), s);
    s = RW_Get_Resource(rf, RW_CONFIG_ENTRY, NULL, &i);
    RW_Create_Robot_File_Entry(e, RW_CONFIG_ENTRY, i, sizeof(char), s);
    /* Write out robot file */
    RW_Write_Robot_File(fp, name, hw, e);
    /* Cleanup */
    RW_Free_Robot_File(rf);
    RW_Free_Robot_File_Entry(e);
    fclose(fp);
}

int main( int argc, char **argv ) {
    FILE *fp, *fp2;
    RW_Robo_Op *code;
    RW_Config_File cf;
    RW_Robot_File *rf;
    char *s;
    size_t code_size, code_length;
    int err_l;
    if( argc < 3 ) {
        fprintf(stdout, "Usage, %s output source [config]\n", argv[0]);
        return 0;
    }
    fp = fopen(argv[2], "r");
    fp2 = NULL;
    if( !fp ) {
        fprintf(stdout, "Failed to open source file.\n");
        return -1;
    }
    if( check_for_robot(fp) ) {
        fclose(fp);
        rf = RW_Open_Robot(argv[2]);
        if( rf == NULL ) {
            fprintf(stdout, "Failed to open robot file.\n");
            return -1;
        }
        s = RW_Get_Resource(rf, RW_CODE_SOURCE, NULL, &code_length);
        if( s == NULL ) {
            fprintf(stdout, "Robot file has no source code.\n");
            return -1;
        }
        code = RW_Compile_Robot_s(s, code_length, &code_size);
        if( code ) {
            write_out_bot_r(argv[1], rf, code, code_size);
        }
    }
    else {
        if( argc > 3 ) {
            fp2 = fopen(argv[3], "r");
            if( !fp2 ) {
                fprintf(stdout, "Failed to open configuration file.\n");
            }
            err_l = RW_Read_Config_File_f(fp2, &cf);
            if( err_l ) {
                fprintf(stdout, "Error reading configuration file on line %d.\n", err_l);
                return -1;
            }
        }
        code = RW_Compile_Robot_f(fp, &code_size);
        if( code ) {
            write_out_bot_f(argv[1], code, code_size, &cf, fp, fp2);
        }
    }
    if( code == NULL ) {
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
