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
#ifndef _SAVERESTORE_H
#define _SAVERESTORE_H
/*****************************************************************************/
#ifdef _WINDOWS

#include <stdio.h>
#define FREAD(x)\
	fread(x,sizeof(x),1,f)
#define FWRITE(x)\
	fwrite(x,sizeof(x),1,f)
#define FREADN(x,n)\
	fread(x,n,1,f)
#define FWRITEN(x,n)\
	fwrite(x,n,1,f)
#define FGETB(x)\
	x=fgetc(f)
#define FPUTB(x)\
	fputc(x,f)
#define FGETW(x)\
	x=(S16)(fgetc(f)|(fgetc(f)<<8))
#define FSKIPW()\
	fgetc(f);fgetc(f)
#define FPUTW(x)\
	fputc(((S16)x)&0xFF,f);\
	fputc(((S16)x)>>8,f)

#define OPEN_SAVE_FILE()\
	if((f=fopen("gbagi.sav","rb+") )==NULL)return FALSE;
#define CLOSE_SAVE_FILE()\
	fclose(f);
#define SAVE_SEEK_SET(x)\
    	fseek(f,x,SEEK_SET)
#define SAVE_SEEK_CUR(x)\
    	fseek(f,x,SEEK_CUR)

#else

#define GAMEPAK_RAM  ((U8*)0x0E000000)

#define OPEN_SAVE_FILE()\
	pSaveMem = GAMEPAK_RAM
#define CLOSE_SAVE_FILE()\
	;

#define SAVE_SEEK_SET(x)\
	pSaveMem = GAMEPAK_RAM+(x)
#define SAVE_SEEK_CUR(x)\
    	pSaveMem+=x
                    
#define FREAD(x)\
	SRamMemCpy((U8*)x,pSaveMem,sizeof(x));\
    pSaveMem+=sizeof(x)
#define FWRITE(x)\
	SRamMemCpy(pSaveMem,(U8*)x,sizeof(x));\
    pSaveMem+=sizeof(x)
#define FREADN(x,n)\
	SRamMemCpy((U8*)x,pSaveMem,n);\
    pSaveMem+=n
#define FWRITEN(x,n)\
	SRamMemCpy(pSaveMem,(U8*)x,n);\
    pSaveMem+=n
#define FGETB(x)\
	x=*pSaveMem++
#define FPUTB(x)\
	*pSaveMem++ = x
#define FGETW(x)\
	x=(S16)(pSaveMem[0]|(pSaveMem[1]<<8));\
    pSaveMem+=2
#define FSKIPW()\
	pSaveMem+=2 
#define FPUTW(x)\
	*pSaveMem++ = ((S16)x)&0xFF;\
	*pSaveMem++ = ((S16)x)>>8
#endif
extern const char szSaveHeader[16];

#define MAX_SAVENAME_LEN	23
extern char szSaveName[MAX_SAVENAME_LEN+1],szAutoSave[MAX_SAVENAME_LEN+1];

/*****************************************************************************/
void InitSaveRestore(void);
BOOL ExecuteSaveDialog(const char *szTitle, const char *szType);
BOOL SaveGame(void);
BOOL RestoreGame(void);
void SRamMemCpy(U8 *a, U8 *b, int len);
/*****************************************************************************/
#endif
/*****************************************************************************/
