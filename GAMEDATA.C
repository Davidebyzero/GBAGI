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
#include "gbagi.h"
#include "gamedata.h"
#include "text.h"
#include "errmsg.h"
#include "menu.h"
#include "system.h"
#include "picture.h"
#include "screen.h"  
#include "commands.h"  
/*****************************************************************************/
U8 *volData;

	// *"dir"
U8 **viewDir;
U8 **picDir;
U8 **logDir;
U8 **sndDir;

	// "object"
char **objNames;
char *objNameData;	// the raw data pointed to by "objNames[256]"
U8 *objRoomsStart;

	// "words.tok"
char **words;		// a-z pointers
char *wordData;

U8 *wordFlags,**logWords;
GAMEENT *GameEnts;
//U32 totalWords;
int totalGames;

extern void *__agi_gamedata_start;
/*****************************************************************************/
char s[100];
#define IDSIZE	24
const char agiid[]="GBAGI 1.0 '''BRIPRO'''\0";  // format still compatible with 1.0, so no reason to change the header

VERTYPE AGIVER;

/*****************************************************************************/
S16 wnGameSelectProc(WND *w, U16 msg, U16 wParam, U32 lParam);

WND wnGameSelect = {
	NULL,NULL,NULL,NULL,
	88,2,152,146,
	{0,0,0,0},{0,0,0,0},
	"Game Select",
	0,
	0,
	wsRESIZABLE|wsTITLE|wsSELECTABLE,
	(WNPROC)wnGameSelectProc
};
WND lbGames = {
	NULL,NULL,&wnGameSelect,NULL,
	0,0,142,106,
	{0,0,0,0},{0,0,0,0},
	"Items",
	wnLISTBOX,
	0,
	wsSELECTABLE|wsEXTFIXED,
	(WNPROC)wnGameSelectProc
};
WND sbGames;

WND bnAbout = {
	NULL,NULL,&wnGameSelect,NULL,
	26,108,40,16,
	{0,0,0,0},{0,0,0,0},
	"About",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	(WNPROC)wnGameSelectProc
};
WND bnSelectGame = {
	NULL,NULL,&wnGameSelect,NULL,
	66,108,76,16,
	{0,0,0,0},{0,0,0,0},
	"Select Game",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	(WNPROC)wnGameSelectProc
};

U32 *b;

/*****************************************************************************/
void ExecuteGameSelectDialog(void);
void ExecuteGameSelectDialog()
{
	int i;
    AddWindow(&wnGameSelect);
    AddWindow(&bnAbout);
    AddWindow(&bnSelectGame);

	WndStopUpdate(&lbGames);
    ListBoxClear(&lbGames);
    AddWindow(&lbGames);
    for(i=0;i<totalGames;i++)
		ListBoxAdd(&lbGames,((GAMEENT*)(b+(i*20)))->name);

	ListBoxSetScrollbar(&lbGames,&sbGames);
	ListBoxSelect(&lbGames,0);
	WndStartUpdate(&lbGames);

	WinGUIDoit();
}
/*****************************************************************************/
S16 wnGameSelectProc(WND *w, U16 msg, U16 wParam, U32 lParam)
{
	U8 *p;
	switch(msg) {
    	case wmBUTTON_CLICK:
        	if(w==&bnAbout) {
                cVersion();
                RedrawAllWindows();
                break;
            }
    	case wmLISTBOX_CLICK:
        	if(wParam==KEY_ENTER||wParam==KEY_START) {
	           	GameEnts=((GAMEENT*)(b+(lbGames.ext.listbox.itemActive->index*20)));
             	WndDispose(&wnGameSelect);
        	}
            break;
    }
	return TRUE;
}
/*****************************************************************************/
BOOL GameDataInit()
{
    int tw,i;
#ifdef _WINDOWS
	U32 *p,*pend;
    LISTITEM *li;
	FILE *f=fopen("E:\\agigames.gba"//myagigames.gba"//gbagi081beta-sq2.gba"//
    	,"rb");
    long l;
    U8 *buf;
    fseek(f,0,SEEK_END);
    l = ftell(f)-BASEx0X;
    fseek(f,BASEx0X,SEEK_SET);
    buf = (U8*)malloc(l);
    fread(buf,l,1,f);
    fclose(f);
	#define _BASE (buf+0x20)
    #define bb(x) (buf+(b[x]-BASE80X))
    #define bn(x) (((U8*)b-0x20)[x])
    if(strcmp((char*)buf,agiid)!=0)
    	ErrorMessage(0,"Game data invalid! Signature check failed!");
#else
	U32 BASE80X = ((U32)&__agi_gamedata_start + AGI_DATA_ALIGNMENT-1) & -AGI_DATA_ALIGNMENT;
	#define _BASE (BASE80X+0x20)
    #define bb(x) (b)[x]
    #define bn(x) (((U8*)BASE80X)[x])
    if(strcmp((char*)BASE80X,agiid)!=0) return FALSE;
#endif

	b=(U32*)_BASE;

    totalGames		= bn(24);

    GameEnts = (GAMEENT*)(b);

    if(totalGames > 1) {
    	ClearControllers();
        cStatusLineOff();
        ShowPic();
    	ExecuteGameSelectDialog();
    }

#ifdef _WINDOWS
    GameEnts->volData	= (U8*)(buf+((U32)GameEnts->volData-BASE80X));
    GameEnts->viewDir	= (U8**)(buf+((U32)GameEnts->viewDir-BASE80X));
    GameEnts->picDir	= (U8**)(buf+((U32)GameEnts->picDir-BASE80X));
    GameEnts->logDir	= (U8**)(buf+((U32)GameEnts->logDir-BASE80X));
    GameEnts->sndDir	= (U8**)(buf+((U32)GameEnts->sndDir-BASE80X));
    GameEnts->objNameData	= (char*)(buf+((U32)GameEnts->objNameData-BASE80X));
    GameEnts->objNames	= (char**)(buf+((U32)GameEnts->objNames-BASE80X));
    GameEnts->objRoomsStart	= (U8*)(buf+((U32)GameEnts->objRoomsStart-BASE80X));
    GameEnts->wordData	= (char*)(buf+((U32)GameEnts->wordData-BASE80X));
    GameEnts->words		= (char**)(buf+((U32)GameEnts->words-BASE80X));
    GameEnts->wordFlags = (char*)(buf+((U32)GameEnts->wordFlags-BASE80X));
    GameEnts->logWords	= (U8**)(buf+((U32)GameEnts->logWords-BASE80X));
#endif

    AGIVER.major = GameEnts->hdr[3];
    AGIVER.minor = bGetW(GameEnts->hdr+4);

    volData = GameEnts->volData;
    viewDir = GameEnts->viewDir;
    picDir = GameEnts->picDir;
    logDir = GameEnts->logDir;
    sndDir = GameEnts->sndDir;
    objNameData = GameEnts->objNameData;
    objNames = GameEnts->objNames;
    objRoomsStart = GameEnts->objRoomsStart;
    wordData = GameEnts->wordData;
    words = GameEnts->words;
    wordFlags = GameEnts->wordFlags;
    logWords = GameEnts->logWords;

#ifdef _WINDOWS
    p = (U32*)logDir;
    pend = (U32*)objNameData;
    while(p<pend) {
   		if(*p)*p = (U32)( (buf)+ (((U32)*p)-BASE80X) );
        p++;
    }
    p = (U32*)objNames;
    pend = (U32*)objRoomsStart-3;
    while(p<pend) {
    	if(*p)*p = (U32)(buf+((*p)-BASE80X));
        p++;
    }
    p = (U32*)words;
    pend = (U32*)wordFlags;
    while(p<pend) {
   		if(*p)
        	*p = (U32)( (buf)+ (((U32)*p)-BASE80X) );
        p++;
    }
    p = (U32*)logWords;
    tw = 256;
    while(tw) {
    	if(*p)*p = (U32)(buf+((*p)-BASE80X));
        p++;
        tw--;
    }
#endif
	return TRUE;
}
/*****************************************************************************/

