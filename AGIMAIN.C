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
#include "views.h"
#include "logic.h"
#include "commands.h"
#include "input.h"
#include "status.h"
#include "system.h"
#include "screen.h"   
#include "picture.h"
#include "text.h"
#include "menu.h"   
#include "invobj.h"  
#include "agimain.h"     
#include "parse.h"   
#include "variables.h"
#include "gamedata.h"
#include "saverestore.h"
/*****************************************************************************/
BOOL PLAYER_CONTROL, TEXT_MODE, WINDOW_OPEN, REFRESH_SCREEN, MENU_SET, INPUT_ENABLED, QUIT_FLAG;
BOOL SOUND_ON, PIC_VISIBLE, PRI_VISIBLE, STATUS_VISIBLE, VOBJ_BLOCKING,WALK_HOLD;
U8 oldScore;
U8 horizon; 
U8 picNum;
U8 minRow,inputPos,statusRow;
U8 textColour,textAttr,textRow,textCol;
int minRowY,ticks;
RECT8 objBlock;
char cursorChar;
char szGameID[MAX_ID_LEN+1];
int pushedScriptCount, scriptCount;

U8 *pSnds[4],*sndBuf;
int sndFlag,sndWaits[4];
/*****************************************************************************/
void InitSound()
{	
	sndBuf = NULL;
	sndFlag=-1;
}
void TIMER2(void);
void StartSound(int num, int flag)
{		
	U8 *p = (U8*)sndDir[num]+5;
	int i; 
    if(!sndDir[num]) {
    	SetFlag(flag);
    	return;
    }

	StopSound();

	sndFlag = flag;
    ResetFlag(sndFlag);

	for(i=0;i<4;i++) {
		pSnds[i] = p + bGetW(p+(i<<1));
		sndWaits[i] = 0;
	}	
	
#ifndef _WINDOWS
	//TIMER2();
#endif
	sndBuf = p + bGetW(p);
}
void StopSound()
{
#ifndef _WINDOWS 
	REG_SOUND1CNT_L=0;
	REG_SOUND1CNT_H=0;
	REG_SOUND1CNT_X=SOUND1INIT+0;	
	REG_SOUND1CNT_X=0;			
	REG_SOUND2CNT_L=0;
	REG_SOUND2CNT_H=SOUND1INIT+0;
	REG_SOUND2CNT_H=0;		
	REG_SOUND3CNT_L=0;
	REG_SOUND3CNT_H=SOUND1INIT+0;
	REG_SOUND3CNT_H=0;
	REG_SOUND3CNT_X=0;
	REG_SOUND4CNT_L=0;
	REG_SOUND4CNT_H=SOUND1INIT+0;
	REG_SOUND4CNT_H=0;
#endif
    if(sndBuf||(!TestFlag(fSOUND)&&sndFlag!=-1)) {
    	SetFlag(sndFlag);
        sndFlag=-1;
		sndBuf=NULL;
    }
}
/*****************************************************************************/
BOOL AGIInit(BOOL RESTART)
{
	strcpy(szGameID,"NO_NAME");

	PLAYER_CONTROL	= TRUE;
	INPUT_ENABLED	= FALSE;
	TEXT_MODE		= FALSE;
    STATUS_VISIBLE	= FALSE;
    VOBJ_BLOCKING	= FALSE;
    WINDOW_OPEN		= FALSE;
    REFRESH_SCREEN	= FALSE;
    PIC_VISIBLE		= FALSE;
    PRI_VISIBLE		= FALSE;
    WALK_HOLD		= FALSE;
    MENU_SELECTABLE	= TRUE;

    scriptCount 	= 0;

    ydiff 			= 0;
	minRow 			= 1;
    minRowY			= (minRow*(SCREEN_WIDTH*CHAR_HEIGHT));
    inputPos		= 22;
    statusRow		= 0;

    textColour		= 0x0F;
    textAttr		= 0;

    msgX			= -1;
    msgY			= -1;
    maxWidth		= -1;

	ClearVars();
	ClearFlags();
	ClearControllers();
    if(!RESTART)
    	ClearControlKeys();

	vars[vCOMPUTER]		= 0; // PC
	vars[vMONTIOR]	= 3; // EGA
	vars[vSOUNDTYPE]		= 1; // PC
	vars[vMAXINPUT]		= MAX_STRINGS_LEN+1;
	vars[vMEMORY]		= 10;

	SetFlag(fNEWROOM);

	InitSound();
    InitLogicSystem();
	InitViewSystem();
	InitPicSystem(TRUE);
	InitObjSystem();
	InitParseSystem();
    InitSaveRestore();
    if(!RESTART)
    	InitMenuSystem();

    SOUND_ON	= TRUE;
	SetFlag(fSOUND);

    return TRUE;
}
/*****************************************************************************/
void AGIShutDown()
{
    FreeMenuSystem();
}
/*****************************************************************************/
//#define SKIPTOSCREEN 18
void AGIMain()
{
	int i;
#ifdef SKIPTOSCREEN
	 int m=1;
#endif
#ifndef _WINDOWS
	REG_TM1CNT = TIME_FREQUENcy1024 | TIME_ENABLE;
	REG_TM1D = 0;
#endif
	for (;;) {
		ClearControllers();

		ResetFlag(fPLAYERCOMMAND);
		ResetFlag(fSAIDOK);
		vars[vKEYPRESSED]	= 0;
		vars[vUNKWORD]		= 0;

		DoDelayNPoll();
        if(QUIT_FLAG) break;
    	SystemDoit();

		if(PLAYER_CONTROL)
			ViewObjs[0].direction = vars[vEGODIR];
		else
			vars[vEGODIR] = ViewObjs[0].direction;

		CalcVObjsDir();

		oldScore = vars[vSCORE];
		SOUND_ON = TestFlag(fSOUND);

#ifdef SKIPTOSCREEN
		if(vars[0]==25) {
        if(m==1) {
        	ViewObjs[0].x = 10;
        	ViewObjs[0].y = 150;
         	NewRoom(SKIPTOSCREEN);
        }
        if(m<2) m++;}
#endif
		while(!CallLogic(0)) {
        	if(QUIT_FLAG) break;
			vars[vUNKWORD]		= 0;
			vars[vOBJBORDER]		= 0;
			vars[vOBJECT]		= 0;
			ResetFlag(fPLAYERCOMMAND);
			oldScore = vars[vSCORE];
		}

		ViewObjs[0].direction = vars[vEGODIR];

		if( (oldScore!=vars[vSCORE]) || (TestFlag(fSOUND)!=SOUND_ON) )
			WriteStatusLine();

		vars[vOBJBORDER]		= 0;
		vars[vOBJECT]		= 0;
		ResetFlag(fNEWROOM);
		ResetFlag(fRESTART);
		ResetFlag(fRESTORE);

		if(!TEXT_MODE)
			UpdateGfx();
	}      
    StopSound();
}

/*****************************************************************************/

/*****************************************************************************/
