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
#ifndef _AGIMAIN_H
#define _AGIMAIN_H
/*****************************************************************************/

BOOL AGIInit(BOOL RESTART);
void AGIInitVars();
void AGIShutDown(void);
void AGIMain(void);
BOOL IsWalkHoldActive(void);

/*****************************************************************************/
extern BOOL PLAYER_CONTROL, TEXT_MODE, WINDOW_OPEN, REFRESH_SCREEN, MENU_SET, INPUT_ENABLED;
extern BOOL SOUND_ON,PIC_VISIBLE,PRI_VISIBLE,STATUS_VISIBLE, VOBJ_BLOCKING,WALK_HOLD,QUIT_FLAG;
extern BOOL MENU_ACTIVE;
extern U8 oldScore;
extern U8 horizon;
extern U8 picNum;
extern U8 minRow,inputPos,statusRow;
extern U8 textColour,textAttr,textRow,textCol;
extern int minRowY,ticks;
extern RECT8 objBlock;
extern char cursorChar;

#define MAX_ID_LEN 7
extern char szGameID[MAX_ID_LEN+1];

extern U8 *pSnds[4],*sndBuf;
extern int sndFlag,sndWaits[4];

void StartSound(int num, int flag);
void StopSound(void);
void StopLegacySoundEffectsOnly(void);
void InitSound(void);
BOOL IsCurrentSoundLikelyMusic(void);
BOOL UseOriginalForCurrentSound(void);
U8 GetCurrentSoundNumber(void);
U8 GetCurrentMusicNumber(void);
U8 GetCurrentSoundDoneFlag(void);
U8 GetCurrentMusicDoneFlag(void);
void ResumeCurrentAudioPlayback(void);

#include "enhanced_audio.h"
/*****************************************************************************/
#endif
/*****************************************************************************/
