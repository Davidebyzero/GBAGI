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
#include "pcm_music.h"
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
static BOOL s_current_sound_likely_music;
static BOOL s_current_sound_use_original;
static BOOL s_first_sound_after_boot;
static U16 s_current_sound_payload_len;
static U8 s_current_sound_num;
static U8 s_current_sound_done_flag;
static U8 s_current_music_num;
static U8 s_current_music_done_flag;

#define FIRST_SOUND_ORIGINAL_PAYLOAD_MAX 128
/*****************************************************************************/
static BOOL SoundChannelHasAudibleTone(const U8 *channel)
{
	U16 len;

	if(!channel)
		return FALSE;

	len = (U16)(channel[0] + (channel[1] << 8));
	if(len == 0xFFFF)
		return FALSE;
	if((channel[4] & 0x0F) == 0x0F)
		return FALSE;
	return ((((U16)channel[2] & 0x3F) << 4) | (U16)(channel[3] & 0x0F)) != 0;
}
/*****************************************************************************/
static BOOL DetectLikelyMusic(U8 *base)
{
	int active_tones;
	int sustained_tones;
	int i;

	active_tones = 0;
	sustained_tones = 0;
	for(i = 0; i < 3; i++) {
		U8 *channel;
		U16 len;

		channel = base + bGetW(base + (i << 1));
		if(!SoundChannelHasAudibleTone(channel))
			continue;

		active_tones++;
		len = (U16)(channel[0] + (channel[1] << 8));
		if(len >= 8)
			sustained_tones++;
	}

	return ((active_tones >= 3) || ((active_tones >= 2) && (sustained_tones >= 1)));
}
/*****************************************************************************/
static BOOL IsSq2OriginalCompatSound(U8 sound_num)
{
	switch(sound_num) {
		case 40:
		case 60:
		case 61:
			return TRUE;
	}
	return FALSE;
}
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
static BOOL IsLSL1GameForPCM(void)
{
	if ((strcmp(szGameID, "LLLLL") == 0) || (strcmp(szGameID, "LSL1") == 0)) {
		return TRUE;
	}

	if (GameEnts && GameEnts->name) {
		if (strncmp(GameEnts->name, "Leisure Suit Larry", 18) == 0) {
			return TRUE;
		}
		if (strncmp(GameEnts->name, "Larry", 5) == 0) {
			return TRUE;
		}
	}

	return FALSE;
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
	s_current_sound_likely_music = FALSE;
	s_current_sound_use_original = FALSE;
	s_first_sound_after_boot = TRUE;
	s_current_sound_payload_len = 0;
	s_current_sound_num = 0xFF;
	s_current_sound_done_flag = 0xFF;
	s_current_music_num = 0xFF;
	s_current_music_done_flag = 0xFF;
	InitAudioMode();
}
void TIMER2(void);
static void StopLiveSoundEffects(void)
{
#ifndef _WINDOWS 
	StopLegacyAudioHardware();
#endif
	if(sndBuf||(!TestFlag(fSOUND)&&sndFlag!=-1)) {
    	SetFlag(sndFlag);
        sndFlag=-1;
		sndBuf=NULL;
		s_current_sound_likely_music = FALSE;
		if(s_first_sound_after_boot)
			s_first_sound_after_boot = FALSE;
		s_current_sound_use_original = FALSE;
		s_current_sound_payload_len = 0;
		s_current_sound_num = 0xFF;
		s_current_sound_done_flag = 0xFF;
    }
}

void StopLegacySoundEffectsOnly(void)
{
	StopLiveSoundEffects();
}

void StartSound(int num, int flag)
{		
	U8 *resource = (U8*)sndDir[num];
	U8 *p = (U8*)sndDir[num]+5;
	int i; 
	BOOL likely_music;
	BOOL has_pcm_mapping;

    if(!sndDir[num]) {
    	SetFlag(flag);
    	return;
    }

	likely_music = DetectLikelyMusic(p);
	if (IsHybridMusicBackendActive() && (num == 21)) {
		StopLiveSoundEffects();
		if (StartLSL1Sound21PCMMusic((U8)flag)) {
			s_current_music_num = 21U;
			s_current_music_done_flag = (U8)flag;
			return;
		}
	}
	has_pcm_mapping = (BOOL)(IsHybridMusicBackendActive() && HasPCMMusicForSound(szGameID, (U8)num));
	if (has_pcm_mapping) {
		StopLiveSoundEffects();
		if (StartPCMMusicForSound(szGameID, (U8)num, (U8)flag)) {
			s_current_music_num = (U8)num;
			s_current_music_done_flag = (U8)flag;
			return;
		}
	}
	if (IsHybridMusicBackendActive() && likely_music && StartPCMMusicForSound(szGameID, (U8)num, (U8)flag)) {
		s_current_music_num = (U8)num;
		s_current_music_done_flag = (U8)flag;
		return;
	}

	StopLiveSoundEffects();

	sndFlag = flag;
    ResetFlag(sndFlag);
	s_current_sound_done_flag = (U8)flag;
	s_current_sound_num = (U8)num;
	s_current_sound_payload_len = bGetW(resource + 3);
	s_current_sound_likely_music = likely_music;
	s_current_sound_use_original = (BOOL)(s_current_sound_payload_len <= FIRST_SOUND_ORIGINAL_PAYLOAD_MAX);
	if((GameEnts && GameEnts->name) &&
		((strncmp(GameEnts->name, "Space Quest 2", 13) == 0) ||
		 (strncmp(GameEnts->name, "Space Quest II", 14) == 0)) &&
		IsSq2OriginalCompatSound(s_current_sound_num))
		s_current_sound_use_original = TRUE;
	for(i=0;i<4;i++) {
		pSnds[i] = p + bGetW(p+(i<<1));
		sndWaits[i] = 0;
	}	
	
#ifndef _WINDOWS
	//TIMER2();
#endif
	sndBuf = pSnds[0] ? pSnds[0] : p;
}
void StopSound()
{
	StopLiveSoundEffects();
	StopPCMMusic();
	s_current_music_num = 0xFF;
	s_current_music_done_flag = 0xFF;
}
/*****************************************************************************/
BOOL IsCurrentSoundLikelyMusic(void)
{
	return s_current_sound_likely_music;
}
/*****************************************************************************/
BOOL UseOriginalForCurrentSound(void)
{
	return s_current_sound_use_original;
}
/*****************************************************************************/
U8 GetCurrentSoundNumber(void)
{
	return s_current_sound_num;
}
/*****************************************************************************/
U8 GetCurrentMusicNumber(void)
{
	return s_current_music_num;
}
/*****************************************************************************/
U8 GetCurrentSoundDoneFlag(void)
{
	return s_current_sound_done_flag;
}
/*****************************************************************************/
U8 GetCurrentMusicDoneFlag(void)
{
	return s_current_music_done_flag;
}
/*****************************************************************************/
void ResumeCurrentAudioPlayback(void)
{
	if (!TestFlag(fSOUND)) {
		return;
	}

	if (s_current_music_num != 0xFFU) {
		StartSound((int)s_current_music_num, (int)s_current_music_done_flag);
		return;
	}

	if (s_current_sound_num != 0xFFU) {
		StartSound((int)s_current_sound_num, (int)s_current_sound_done_flag);
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
	vars[vDELAY]		= 1; // start on "fast"
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
		{
			U8 oldBoostMask;
			U16 oldBoostDelayResultTenths;

			oldBoostMask = GetActiveBoostMask();
			oldBoostDelayResultTenths = GetCurrentGameplayDelayResultTenths();
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

		if( (oldScore!=vars[vSCORE]) || (TestFlag(fSOUND)!=SOUND_ON) ||
			(oldBoostMask != GetActiveBoostMask()) ||
			(oldBoostDelayResultTenths != GetCurrentGameplayDelayResultTenths()) )
			WriteStatusLine();
		}

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

