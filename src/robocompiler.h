/*
 * robocompiler.h
 *
 * By Steven Smith
 */

#ifndef ROBOCOMPILER_H_
#define ROBOCOMPILER_H_

#include <stdio.h>
#include "robocode.h"

RW_Robo_Op * RW_Compile_Robot_f( FILE *fp, size_t *length );

RW_Robo_Op * RW_Compile_Robot_s( char *s, size_t size, size_t *length );

const char * RW_Get_Compiler_Error( int *error_line );

#endif
