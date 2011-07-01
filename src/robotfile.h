/*
 * robotfile.h
 *
 * By Steven Smith
 */

#ifndef ROBOTFILE_H_
#define ROBOTFILE_H_

#include "robocode.h"

#define magic_field 0x524F424F /* ROBO */

#define version_number 1

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
    unsigned int size;
    unsigned char *name;
    unsigned char *bytes;
} RW_Robot_File_Entry;

typedef struct {
    unsigned int magic; /* Magic field 'robo' */
    unsigned int version;
    RW_Hardware_Spec hardware;
} RW_Robot_File;

#endif
