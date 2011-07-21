/*
 * robotfile.h
 *
 * By Steven Smith
 */

#ifndef ROBOTFILE_H_
#define ROBOTFILE_H_

#include <stdio.h>
#include <stdint.h>
#include "robocode.h"

#define RW_MAGIC { 'R', 'O', 'B', 'O' }
#define RW_CODE_ENTRY "robocode"
#define RW_VERSION 2

typedef struct {
    unsigned char energy;
    unsigned char damage;
    unsigned char shield;
    unsigned char bullet;
    unsigned char probes;
    unsigned char negenergy;
    unsigned char hellbore;
    unsigned char mine;
    unsigned char missile;
    unsigned char tacnuke;
    unsigned char stunner;
} RW_Hardware_Spec;

typedef struct {
    char magic[4];
    uint32_t version;
    char *name;
    RW_Hardware_Spec hardware;
} RW_Robot_File_Header;

typedef struct {
    char *name;
    uint32_t length;
    uint32_t size;
    void *data;
} RW_Robot_File_Index;

typedef struct RW_Robot_Index_List {
    RW_Robot_File_Index i;
    struct RW_Robot_Index_List *next;
} RW_Robot_Index_List;

typedef struct {
    RW_Robot_File_Header *hdr;
    RW_Robot_Index_List *idx;
} RW_Robot_File;

typedef struct RW_Robot_File_Entry {
    char *name;
    size_t length;
    size_t size;
    void *data;
    struct RW_Robot_File_Entry *next;
} RW_Robot_File_Entry;

/* Reading robot files */
RW_Robot_File * RW_Open_Robot( char *fname );

void * RW_Get_Resource( RW_Robot_File *rf, char *key, size_t *size, size_t *length );

void * RW_Get_Resource_Copy( RW_Robot_File *rf, char *key, size_t *size,
    size_t *length );

char * RW_Get_Robot_Name_From_File( RW_Robot_File *rf );

char * RW_Get_Robot_Name_From_File_Copy( RW_Robot_File *rf );

RW_Hardware_Spec RW_Get_Hardware_From_File( RW_Robot_File *rf );

void RW_Free_Robot_File( RW_Robot_File *rf );

/* Writing robot files */
int RW_Write_Robot_File( FILE *fp, char *name, RW_Hardware_Spec hw,
    RW_Robot_File_Entry *entry );

RW_Robot_File_Entry * RW_Create_Robot_File_Entry( RW_Robot_File_Entry *entry,
    char *name, size_t length, size_t size, void *data );

void RW_Free_Robot_File_Entry( RW_Robot_File_Entry *entry );

#endif
