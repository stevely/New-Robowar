/*
 * robotfile.c
 *
 * By Steven Smith
 */

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "robotfile.h"

int RW_Check_Magic( char *magic ) {
    char m[] = RW_MAGIC;
    int i;
    for( i = 0; i < 4; i++ ) {
        if( *magic != m[i] ) {
            return 0;
        }
        magic++;
    }
    return 1;
}

static void clear_index( RW_Robot_Index_List *l ) {
    RW_Robot_Index_List *l_dead;
    fprintf(stderr, "Failure: Clearing index\n");
    while( l ) {
        if( l->i.data ) {
            free(l->i.data);
        }
        if( l->i.name ) {
            free(l->i.name);
        }
        l_dead = l;
        l = l->next;
        free(l_dead);
    }
}

static RW_Robot_Index_List * build_index( FILE *fp ) {
    RW_Robot_Index_List *l_head, *l;
    int c, i;
    char buf[32];
    void *data;
    l_head = NULL;
    l = NULL;
    uint32_t tmp;
    /* We signal the end of the index with a single null \0 character */
    while( (c = fgetc(fp)) != 0 && c != EOF ) {
        i = 1;
        buf[0] = (char)c;
        if( l_head == NULL ) {
            l_head = (RW_Robot_Index_List*)malloc(sizeof(RW_Robot_Index_List));
            l_head->next = NULL;
            if( l_head == NULL ) {
                clear_index(l_head);
                fclose(fp);
                return NULL;
            }
            l = l_head;
        }
        else {
            l->next = (RW_Robot_Index_List*)malloc(sizeof(RW_Robot_Index_List));
            if( l == NULL ) {
                clear_index(l_head);
                fclose(fp);
                return NULL;
            }
            l = l->next;
            l->next = NULL;
        }
        /* Step 1: Get resource name */
        while( i < 31 && (c = fgetc(fp)) != 0 && c != EOF ) {
            buf[i] = (char)c;
            i++;
        }
        buf[i] = 0;
        l->i.name = (char*)malloc(sizeof(char) * (i+1));
        if( l->i.name == NULL ) {
            clear_index(l_head);
            fclose(fp);
            return NULL;
        }
        for( ; i >= 0; i-- ) {
            l->i.name[i] = buf[i];
        }
        /* Step 2: Get data length/size */
        /*
        fread(&(l->i.length), sizeof(uint32_t), 1, fp);
        fread(&(l->i.size), sizeof(uint32_t), 1, fp);
        */
        fread(&tmp, sizeof(uint32_t), 1, fp);
        l->i.length = tmp;
        fread(&tmp, sizeof(uint32_t), 1, fp);
        l->i.size = tmp;
    }
    if( c == EOF ) {
        clear_index(l_head);
        fclose(fp);
        return NULL;
    }
    /* Step 3: Grab data */
    /* Note: It's not technically necessary to grab the data right away, but it's
       simpler for the initial implementation. */
    l = l_head;
    while( l ) {
        data = malloc(l->i.size * l->i.length);
        if( !data ) {
            clear_index(l_head);
            fclose(fp);
            return NULL;
        }
        if( fread(data, l->i.length, l->i.size, fp) != l->i.size ) {
            fprintf(stderr, "%d\n", l->i.size);
            clear_index(l_head);
            fclose(fp);
            return NULL;
        }
        l->i.data = data;
        l = l->next;
    }
    /* Since we just grab all the data we don't need to hold on to the file pointer */
    fclose(fp);
    return l_head;
}

RW_Robot_File * RW_Open_Robot( char *fname ) {
    FILE *fp;
    RW_Robot_File_Header *hdr;
    RW_Robot_Index_List *idx;
    RW_Robot_File *f;
    char buf[32];
    int i;
    int c;
    /* Step 1: Open file */
    fp = fopen(fname, "rb");
    if( !fp ) {
        return NULL;
    }
    /* Step 2: Check magic number */
    hdr = (RW_Robot_File_Header*)malloc(sizeof(RW_Robot_File_Header));
    if( !hdr ) {
        fclose(fp);
        return NULL;
    }
    f = (RW_Robot_File*)malloc(sizeof(RW_Robot_File));
    if( !f ) {
        free(hdr);
        fclose(fp);
        return NULL;
    }
    fread(&(hdr->magic), sizeof(char), 4, fp);
    if( !RW_Check_Magic(hdr->magic) ) {
        free(hdr);
        fclose(fp);
        return NULL;
    }
    /* Step 3: Check version */
    fread(&(hdr->version), sizeof(uint32_t), 1, fp);
    if( hdr->version != RW_VERSION ) {
        free(hdr);
        fclose(fp);
        return NULL;
    }
    /* Step 4: Get name */
    i = 0;
    while( i < 31 && (c = fgetc(fp)) != 0 && c != EOF ) {
        buf[i] = (char)c;
        i++;
    }
    buf[i] = 0;
    hdr->name = (char*)malloc(sizeof(char) * (i+1));
    if( !hdr->name ) {
        free(hdr);
        fclose(fp);
        return NULL;
    }
    for( ; i >= 0; i-- ) {
        hdr->name[i] = buf[i];
    }
    /* Step 5: Get hardware */
    fread(&(hdr->hardware), sizeof(RW_Hardware_Spec), 1, fp);
    /* Step 6: Build our index */
    idx = build_index(fp);
    if( !idx ) {
        fprintf(stderr, "Building index failed!\n");
        return NULL;
    }
    /* Step 7: Bundle it up and cleanup */
    f->hdr = hdr;
    f->idx = idx;
    return f;
}

void * RW_Get_Resource( RW_Robot_File *rf, char *key, size_t *size, size_t *length ) {
    char *k, *n;
    RW_Robot_Index_List *l;
    if( rf == NULL || key == NULL ) {
        return NULL;
    }
    l = rf->idx;
    /* Cycle through indices until we find what we're looking for */
    while( l ) {
        n = l->i.name;
        k = key;
        while( *n == *k && *n != 0 ) {
            n++;
            k++;
        }
        if( *n == *k ) {
            /* Found it */
            if( length ) {
                *length = (size_t)l->i.length;
            }
            if( size ) {
                *size = (size_t)l->i.size;
            }
            return l->i.data;
        }
        else {
            l = l->next;
        }
    }
    /* Don't have it */
    return NULL;
}

void * RW_Get_Resource_Copy( RW_Robot_File *rf, char *key, size_t *size,
    size_t *length ) {
    void *old, *new;
    /* Need our own size/length variables for malloc because the passed-in ones
       might be null. */
    size_t s, l;
    old = RW_Get_Resource(rf, key, &s, &l);
    new = malloc(s * l);
    memcpy(new, old, s * l);
    if( size ) {
        *size = s;
    }
    if( length ) {
        *length = l;
    }
    return new;
}

char * RW_Get_Robot_Name_From_File( RW_Robot_File *rf ) {
    if( rf == NULL ) {
        return NULL;
    }
    return rf->hdr->name;
}

char * RW_Get_Robot_Name_From_File_Copy( RW_Robot_File *rf ) {
    char *n, *name;
    int i = 1;
    if( rf == NULL ) {
        return NULL;
    }
    n = rf->hdr->name;
    name = rf->hdr->name;
    while( *n ) {
        i++;
        n++;
    }
    n = (char*)malloc(sizeof(char) * i);
    for( ; i > 0; i-- ) {
        n[i-1] = name[i-1];
    }
    return n;
}

RW_Hardware_Spec RW_Get_Hardware_From_File( RW_Robot_File *rf ) {
    RW_Hardware_Spec s = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if( rf && rf->hdr ) {
        return rf->hdr->hardware;
    }
    else {
        return s;
    }
}

void RW_Free_Robot_File( RW_Robot_File *rf ) {
    RW_Robot_Index_List *ril;
    if( rf == NULL ) {
        return;
    }
    /* Step 1: Free the header */
    if( rf->hdr ) {
        if( rf->hdr->name ) {
            free(rf->hdr->name);
        }
        free(rf->hdr);
    }
    /* Step 2: Free the index */
    while( rf->idx ) {
        ril = rf->idx;
        rf->idx = ril->next;
        if( ril->i.name ) {
            free(ril->i.name);
        }
        if( ril->i.data ) {
            free(ril->i.data);
        }
        free(ril);
    }
    /* Step 3: Free the robot file */
    free(rf);
}

int RW_Write_Robot_File( FILE *fp, char *name, RW_Hardware_Spec hw,
    RW_Robot_File_Entry *entry ) {
    char magic[] = RW_MAGIC;
    char *n = name;
    uint32_t version = RW_VERSION;
    RW_Robot_File_Entry *e = entry;
    if( fp == NULL || name == NULL || *name == 0 ) {
        return 1;
    }
    /* Header */
    fwrite(magic, sizeof(char), 4, fp);
    fwrite(&version, sizeof(uint32_t), 1, fp);
    while( *n ) {
        fputc(*n, fp);
        n++;
    }
    fputc(0, fp);
    fwrite(&hw, sizeof(RW_Hardware_Spec), 1, fp);
    /* Index table */
    while( e ) {
        n = e->name;
        while( *n ) {
            fputc(*n, fp);
            n++;
        }
        fputc(0, fp);
        fwrite(&(e->length), sizeof(uint32_t), 1, fp);
        fwrite(&(e->size), sizeof(uint32_t), 1, fp);
        e = e->next;
    }
    fputc(0, fp); /* Final trailing 0 to signal end of index table */
    /* Data */
    e = entry;
    while( e ) {
        fwrite(e->data, e->size, e->length, fp);
        e = e->next;
    }
    return 0;
}

RW_Robot_File_Entry * RW_Create_Robot_File_Entry( RW_Robot_File_Entry *entry,
    char *name, size_t length, size_t size, void *data ) {
    RW_Robot_File_Entry *e;
    if( name == NULL || data == NULL || *name == 0 ) {
        return NULL;
    }
    e = (RW_Robot_File_Entry*)malloc(sizeof(RW_Robot_File_Entry));
    if( e == NULL ) {
        return NULL;
    }
    e->name = name;
    e->length = length;
    e->size = size;
    e->data = data;
    e->next = NULL;
    if( entry == NULL ) {
        return e;
    }
    while( entry->next != NULL ) {
        entry = entry->next;
    }
    entry->next = e;
    return e;
}

/* Note: This is a shallow free. Data beyond the file entry list is not freed. */
void RW_Free_Robot_File_Entry( RW_Robot_File_Entry *entry ) {
    RW_Robot_File_Entry *e;
    while( entry ) {
        e = entry;
        entry = entry->next;
        free(e);
    }
}
