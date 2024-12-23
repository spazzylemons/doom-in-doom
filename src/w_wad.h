//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	WAD I/O functions.
//


#ifndef __W_WAD__
#define __W_WAD__

#include <stdio.h>

#include "doomtype.h"


//
// TYPES
//

//
// WADFILE I/O related stuff.
//

typedef struct lumpinfo_s lumpinfo_t;
typedef int lumpindex_t;

struct lumpinfo_s
{
    char	name[8];
    int		size;
    void       *cache;

    // Used for hash table lookups
    lumpindex_t next;
};


extern lumpinfo_t *lumpinfo;
extern unsigned int numlumps;

void W_Init(void);

lumpindex_t W_CheckNumForName(const char *name);
lumpindex_t W_GetNumForName(const char *name);

int W_LumpLength(lumpindex_t lump);
void W_ReadLump(lumpindex_t lump, void *dest);

void *W_CacheLumpNum(lumpindex_t lump, int tag);
void *W_CacheLumpName(const char *name, int tag);

void W_GenerateHashTable(void);

extern unsigned int W_LumpNameHash(const char *s);

void W_ReleaseLumpNum(lumpindex_t lump);
void W_ReleaseLumpName(const char *name);

#endif
