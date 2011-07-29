/*
 * roboconfig.h
 *
 * By Steven Smith
 */

#ifndef ROBOCONFIG_H_
#define ROBOCONFIG_H_

#include "robotfile.h"

typedef struct RW_Config_File {
    char *key;
    char *value;
    struct RW_Config_File *next;
} RW_Config_File;

char * RW_Config_Lookup( RW_Config_File *cf, char *key );

int RW_Read_Config_File_f( FILE *fp, RW_Config_File *cf );

int RW_Read_Config_File_s( char *s, size_t size, RW_Config_File *cf );

RW_Hardware_Spec RW_Get_HW_From_Config( RW_Config_File *cf );

void RW_Free_Config_File( RW_Config_File *cf );

#endif
