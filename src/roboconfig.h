/*
 * roboconfig.h
 *
 * By Steven Smith
 */

#ifndef ROBOCONFIG_H_
#define ROBOCONFIG_H_

#include "robotfile.h"

char * RW_Config_Lookup( char *key );

int RW_Read_Config_File_f( FILE *fp );

RW_Hardware_Spec RW_Get_HW_From_Config();

#endif
