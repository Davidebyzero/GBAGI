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

/******************************************************************************/
#ifndef _MAKEROM_H
#define _MAKEROM_H
/******************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <string.h>  
#include <stdarg.h>
#include <stdlib.h>

#include "../gbagi.h"
/******************************************************************************/
#define BASE800	0x08000000
#define BASEx0X	0x00020000

#define BASE80X	(BASE800|BASEx0X)

#define IDSIZE	24
extern char agiid[IDSIZE];
/******************************************************************************/
#define LF_INFILE	1
#define LF_MSG 		2

#define SINGLE_DIR	1
#define PACKED_DIRS	2
#define ENCRYPT_OBJ	4
#define AMIGA		8
/******************************************************************************/
#define mFree(x)\
	if(x) free(x);\
	x=NULL

#define ALIGN(f)\
while((offs&3)!=0) {\
     	fputc(0,f);\
        offs++; \
    }
/******************************************************************************/
typedef struct {
	S32	offset;
	S8	vol;
	U16	length;
} DIRENT;

typedef struct {
	U32 flags;
    U16 major,minor;
} GVER;

typedef struct {
	const char *name;
	GVER ver;
} VERLIST;

typedef struct {
	VERLIST *version;
	const char *vID;
	const char *title;
	const char *path;
} GAMEINFO;
               
typedef struct {
	U32 addr;
	U16 group;
	const char *string;
} WORDSET;
/******************************************************************************/
extern GAMEINFO *gi;

extern DIRENT dirs[4][256];
extern const char *dirNames[4];

extern char fname[1024];

extern char *inromName,*outromName,*vocabName;

extern BOOL VUSED[16];
extern U8 *volData;
extern U32 volSize;

extern char *objNames[256];
extern char *objNameData;
extern U8 objRoomsStart[256];

extern U8 *words[26];
extern U8 *vocabData, *wordData;

extern FILE *fout;

extern U32 offs, giPos;
/******************************************************************************/    
int ErrorMessage(const char *s, ...);
U16 fgetw(FILE *f);
U32 fgett(FILE *f);
U32 fgetl(FILE *f);
void fputw(U16 l, FILE *f);
void fputt(U32 l, FILE *f);
void fputl(U32 l, FILE *f);
U16 bGetW(U8 *p);
U16 beGetW(U8 *p);
BOOL LoadDir(BOOL SINGLE, int num);
BOOL PrepDirs(void);
BOOL ProcessDirs(void);
BOOL ProcessObject(void);
U8 *LoadFile(BOOL G_PATH, const char *name, int *len);
int FindWordx(const char *s,U8 *b);
void AddWord(int group, const char *string);
const char *FindWord(int group);
const char *FindWord2(int group);
int FindWordStr(const char *s);
void ClearGroup(int group);
void DoSolidGroups(void);
void DoSolidWords(void);
void DoRemainingWords(void);
int  AlphaSortWords(void);
BOOL ProcessWords(void);
void DumpLog(int num);
void ExecuteIF(void);
BOOL OutputGame(void);
BOOL ProcessGame(GAMEINFO *gmInfo);
void FreeGame(void);
int FindTotalWordsInGroup(int group);
char **FindExtraWordInGroup(int group);

VERLIST *FindAGIVersion(char *filename);
/******************************************************************************/
#endif
/******************************************************************************/