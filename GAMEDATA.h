/***************************************************************************
 *  GBAGI: The Game Boy Advance Adventure Game Interpreter
 *  Copyright (C) 2003,2004 Brian Provinciano, http://www.bripro.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software Foundation,
 *  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

/*****************************************************************************/
#ifndef _GAMEDATA_H
#define _GAMEDATA_H
/*****************************************************************************/

#include "wingui.h"

/*****************************************************************************/
	// "vol."*
extern U8 *volData;

	// *"dir"
extern U8 **viewDir;
extern U8 **picDir;
extern U8 **logDir;
extern U8 **sndDir;

	// "object"
extern char **objNames;
extern char *objNameData;	// the raw data pointed to by "objNames[256]"
extern U8 *objRoomsStart;

	// "words.tok"
extern char **words;		// a-z pointers
extern char *wordData;		// the raw data pointed to by "words[26]"
							// each letter has an array of SZs, terminated by a "" string.

extern U8 *wordFlags,**logWords;
//extern U32 totalWords;

typedef struct {
	U8 major;
    U16 minor;
} VERTYPE;
extern VERTYPE AGIVER;


typedef struct {
	U8 hdr[6];
    char name[26];

    U8 *volData;
	U8 **logDir;
	U8 **picDir;  
	U8 **viewDir;
	U8 **sndDir;
	char *objNameData;
	char **objNames;
	U8 *objRoomsStart;
	char *wordData;
	char **words;
	char *wordFlags;
	U8 **logWords;
} GAMEENT;

extern GAMEENT *GameEnts;
/*****************************************************************************/
BOOL GameDataInit(void);
/*****************************************************************************/
#endif
/*****************************************************************************/
