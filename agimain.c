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
BOOL MENU_ACTIVE;
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
static void SyncKQ4Room1SwimState(void)
{
	VOBJ *ego;
	U8 *row;
	U8 deepColor;
	int x;
	int deepEdge = PIC_WIDTH;
	int gapLen = 0;
	int shoreEdge = -1;
	int deepSwimEdge = 43;
	int deepWadeEdge = 47;
	int beachWalkEdge = 93;
	int footX;
	int rightFootX;

	if(strcmp(szGameID, "KQ4") != 0)
		return;
	if(vars[vROOMNUM] != 1)
		return;

	ego = &ViewObjs[0];
	if((ego->flags & (oDRAWN|oANIMATE)) != (oDRAWN|oANIMATE))
		return;
	if((ego->y < 0) || (ego->y > PIC_MAXY))
		return;

	row = MAKE_PICBUF_PTR(0, ego->y);
	deepColor = row[0] & 0x0F;
	for(x = 0; x < PIC_WIDTH; x++) {
		if(((row[x] & 0xF0) == PRI_WATER) && ((row[x] & 0x0F) == deepColor)) {
			gapLen = 0;
			continue;
		}
		if(((row[x] & 0xF0) == PRI_WATER) && (gapLen < 2)) {
			gapLen++;
			continue;
		}
		deepEdge = x - gapLen;
		if(deepEdge < 0)
			deepEdge = 0;
		break;
	}
	for(x = PIC_WIDTH - 1; x >= 0; x--) {
		if((row[x] & 0xF0) == PRI_WATER) {
			shoreEdge = x;
			break;
		}
	}

	footX = ego->x + (ego->width >> 1);
	rightFootX = ego->x + ego->width - 1;

	if(((ego->view == 5) || (vars[37] == 12)) ? (footX <= deepWadeEdge) : (footX <= deepSwimEdge)) {
		if(ego->view != 5) {
			SetObjView(ego, 5);
			vars[vEGOVIEWNUM] = 5;
		}
		vars[37] = 12;
	} else if(footX >= beachWalkEdge) {
		if((ego->view >= 2) && (ego->view <= 5)) {
			SetObjView(ego, 0);
			vars[vEGOVIEWNUM] = 0;
		}
		if(vars[37] == 12)
			vars[37] = 0;
	} else if((shoreEdge >= 0) && (rightFootX <= shoreEdge)) {
		if(ego->view != 4) {
			SetObjView(ego, 4);
			vars[vEGOVIEWNUM] = 4;
		}
		if(vars[37] == 12)
			vars[37] = 0;
	} else {
		if((ego->view >= 2) && (ego->view <= 5)) {
			SetObjView(ego, 0);
			vars[vEGOVIEWNUM] = 0;
		}
		if(vars[37] == 12)
			vars[37] = 0;
	}
}
/*****************************************************************************/
static BOOL IsPoliceQuestGame(void)
{
	return (GameEnts && (strncmp(GameEnts->name, "Police Quest", 12) == 0));
}
/*****************************************************************************/
static BOOL IsPoliceQuestDriving(void)
{
	if(!IsPoliceQuestGame())
		return FALSE;

	/*
	 * Police Quest driving uses a fixed set of room numbers. Keep the
	 * override explicit so on-foot scenes stay in D-pad mode.
	 */
	return (
		(vars[vROOMNUM] == 10) ||
		(vars[vROOMNUM] == 11) ||
		(vars[vROOMNUM] == 12) ||
		(vars[vROOMNUM] == 13) ||
		(vars[vROOMNUM] == 14) ||
		(vars[vROOMNUM] == 15) ||
		(vars[vROOMNUM] == 16) ||
		(vars[vROOMNUM] == 17) ||
		(vars[vROOMNUM] == 18) ||
		(vars[vROOMNUM] == 19) ||
		(vars[vROOMNUM] == 20) ||
		(vars[vROOMNUM] == 21) ||
		(vars[vROOMNUM] == 22) ||
		(vars[vROOMNUM] == 23) ||
		(vars[vROOMNUM] == 24) ||
		(vars[vROOMNUM] == 25)
	);
}
/*****************************************************************************/
BOOL IsWalkHoldActive(void)
{
	return (WALK_HOLD && !MENU_ACTIVE && !IsPoliceQuestDriving());
}
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
	REG_SOUND2CNT_H=SOUND2INIT+0;
	REG_SOUND2CNT_H=0;		
	REG_SOUND3CNT_L=0;
	REG_SOUND3CNT_H=SOUND3INIT+0;
	REG_SOUND3CNT_H=0;
	REG_SOUND3CNT_X=0;
	REG_SOUND4CNT_L=0;
	REG_SOUND4CNT_H=SOUND4INIT+0;
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
    WALK_HOLD		= TRUE;
    MENU_ACTIVE		= FALSE;
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

    AGIInitVars();

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
void AGIInitVars()
{
	vars[vCOMPUTER]		= 0; // PC
#ifdef FAKE_HERCULES
	vars[vMONTIOR]	= 2; // MONO (Hercules monochrome)
#else
	vars[vMONTIOR]	= 3; // EGA
#endif
	vars[vSOUNDTYPE]		= 1; // PC
	vars[vMAXINPUT]		= MAX_STRINGS_LEN;
	vars[vMEMORY]		= 10;
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
#ifdef SKIPTOSCREEN
	 int m=1;
#endif
#ifndef _WINDOWS
	REG_TM1CNT_H = TIME_FREQUENcy1024 | TIME_ENABLE;
	REG_TM1CNT_L = 0;
#endif
	for (;;) {
		ClearControllers();

		ResetFlag(fPLAYERCOMMAND);
		ResetFlag(fSAIDOK);
		vars[vKEYPRESSED]	= 0;
		vars[vUNKWORD]		= 0;

		DoDelayNPoll();
        if(QUIT_FLAG) break;
		BatterylessUpdateCommitPump();
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

		SyncKQ4Room1SwimState();

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
