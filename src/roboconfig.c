/*
 * roboconfig.c
 *
 * By Steven Smith
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "robotfile.h"

typedef struct kv_list {
    char *key;
    char *value;
    struct kv_list *next;
} kv_list;

static kv_list *kvl = NULL;
static kv_list *kvl_end = NULL;

#define IDENTBUF_SIZE 256
static char kv_buf[IDENTBUF_SIZE + 1];

static char * copy_ident( int kv_pos ) {
    int i;
    char *c;
    c = (char*)malloc(sizeof(char) * (kv_pos + 1));
    for( i = 0; i <= kv_pos; i++ ) {
        c[i] = kv_buf[i];
    }
    return c;
}

static int build_kv_list_f( FILE *fp ) {
    int curr_char;
    int kv_pos;
    int current_line;
    kv_list *k;
    if( fp == NULL ) {
        return -1;
    }
    curr_char = fgetc(fp);
    current_line = 1;
    while( curr_char != EOF ) {
        kv_pos = 0;
        /* Grab key */
        while( curr_char != EOF && curr_char != ' ' && curr_char != '\n'
            && kv_pos < IDENTBUF_SIZE ) {
            kv_buf[kv_pos] = curr_char;
            kv_pos++;
            curr_char = fgetc(fp);
        }
        /* Make sure we grabbed something */
        if( kv_pos == 0 || curr_char == EOF || curr_char == '\n' ) {
            return current_line;
        }
        /* Create our key-value struct */
        kv_buf[kv_pos] = 0;
        k = (kv_list*)malloc(sizeof(kv_list));
        k->next = NULL;
        k->key = copy_ident(kv_pos);
        /* Eat any whitespace between the key and the value */
        while( curr_char == ' ' ) {
            curr_char = fgetc(fp);
        }
        kv_pos = 0;
        if( curr_char == EOF || curr_char == '\n' ) {
            return current_line;
        }
        /* Grab value */
        while( curr_char != EOF && curr_char != '\n' && kv_pos < IDENTBUF_SIZE ) {
            kv_buf[kv_pos] = curr_char;
            kv_pos++;
            curr_char = fgetc(fp);
        }
        /* Insert value into the key-value struct */
        kv_buf[kv_pos] = 0;
        k->value = copy_ident(kv_pos);
        /* Insert our key-value struct into the global list */
        if( kvl == NULL ) {
            kvl = k;
            kvl_end = k;
        }
        else {
            kvl_end->next = k;
            kvl_end = k;
        }
        current_line++;
        curr_char = fgetc(fp);
    }
    return 0;
}

char * RW_Config_Lookup( char *key ) {
    kv_list *k;
    char *c1, *c2;
    if( key == NULL ) {
        return NULL;
    }
    k = kvl;
    while( k ) {
        c1 = key;
        c2 = k->key;
        while( *c1 && *c2 && *c1 == *c2 ) {
            c1++;
            c2++;
        }
        /* Did we match? */
        if( *c1 == *c2 ) {
            return k->value;
        }
        else {
            k = k->next;
        }
    }
    /* Didn't find it */
    return NULL;
}

int RW_Read_Config_File_f( FILE *fp ) {
    int errline;
    kv_list *k;
    errline = build_kv_list_f(fp);
    if( errline ) {
        /* Parsing failed, free everything */
        while( kvl ) {
            k = kvl;
            if( kvl->key ) {
                free(kvl->key);
            }
            if( kvl->value ) {
                free(kvl->value);
            }
            kvl = kvl->next;
            free(k);
        }
        return errline;
    }
    else {
        return 0;
    }
}

RW_Hardware_Spec RW_Get_HW_From_Config() {
    RW_Hardware_Spec hw;
    char *c;
    int i;
    c = RW_Config_Lookup("energy");
    if( c ) {
        i = atoi(c);
        if( i == 100 ) {
            hw.energy = 1;
        }
        else if( i == 150 ) {
            hw.energy = 2;
        }
        else {
            hw.energy = 0;
        }
    }
    else {
        hw.energy = 0;
    }
    c = RW_Config_Lookup("damage");
    if( c ) {
        i = atoi(c);
        if( i == 300 ) {
            hw.damage = 1;
        }
        else if( i == 450 ) {
            hw.damage = 2;
        }
        else {
            hw.damage = 0;
        }
    }
    else {
        hw.damage = 0;
    }
    c = RW_Config_Lookup("shield");
    if( c ) {
        i = atoi(c);
        if( i == 50 ) {
            hw.shield = 1;
        }
        else if( i == 100 ) {
            hw.shield = 2;
        }
        else {
            hw.shield = 0;
        }
    }
    else {
        hw.shield = 0;
    }
    c = RW_Config_Lookup("bullet");
    if( c ) {
        if( strcmp(c, "explosive") == 0 ) {
            hw.bullet = 2;
        }
        else if( strcmp(c, "rubber") == 0 ) {
            hw.bullet = 0;
        }
        else {
            hw.bullet = 1;
        }
    }
    else {
        hw.bullet = 1;
    }
    c = RW_Config_Lookup("probes");
    if( c && strcmp(c, "yes") == 0 ) {
        hw.probes = 1;
    }
    else {
        hw.probes = 0;
    }
    c = RW_Config_Lookup("negenergy");
    if( c && strcmp(c, "yes") == 0 ) {
        hw.negenergy = 1;
    }
    else {
        hw.negenergy = 0;
    }
    c = RW_Config_Lookup("hellbore");
    if( c && strcmp(c, "yes") == 0 ) {
        hw.hellbore = 1;
    }
    else {
        hw.hellbore = 0;
    }
    c = RW_Config_Lookup("mine");
    if( c && strcmp(c, "yes") == 0 ) {
        hw.mine = 1;
    }
    else {
        hw.mine = 0;
    }
    c = RW_Config_Lookup("missile");
    if( c && strcmp(c, "yes") == 0 ) {
        hw.missile = 1;
    }
    else {
        hw.missile = 0;
    }
    c = RW_Config_Lookup("tacnuke");
    if( c && strcmp(c, "yes") == 0 ) {
        hw.tacnuke = 1;
    }
    else {
        hw.tacnuke = 0;
    }
    c = RW_Config_Lookup("tacnuke");
    if( c && strcmp(c, "yes") == 0 ) {
        hw.stunner = 1;
    }
    else {
        hw.stunner = 0;
    }
    return hw;
}
