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
#include "input.h"
#include "views.h"
#include "system.h"
#include "parse.h"
#include "text.h"
#include "wingui.h"
#include "keyboard.h"
#include "agimain.h"
#include "gamedata.h"
#include "pcm_music.h"
#include "lsl1hack.h"
#include "screen.h" // for Y_ADJUST_CL and RedrawScreenAll()
#ifdef _WINDOWS
#include <windows.h>
#endif
/*****************************************************************************/
EVENT tmpEvent, evStopEgo = {EV_DIRECTION, dirNONE};
BTNSTATE btnstate;
/*****************************************************************************/
const KEY keyDirs[]={
	{KEY_UP, 		dirUP		}, {KEY_PGUP,		dirUPRIGHT		},
    {KEY_RIGHT, 	dirRIGHT	}, {KEY_PGDOWN, 	dirDOWNRIGHT	},
    {KEY_DOWN, 		dirDOWN		}, {KEY_END, 		dirDOWNLEFT		},
    {KEY_LEFT, 		dirLEFT		}, {KEY_HOME, 		dirUPLEFT		},
    {KEY_NUMPAD8,	dirUP		}, {KEY_NUMPAD9, 	dirUPRIGHT		},
    {KEY_NUMPAD6, 	dirRIGHT	}, {KEY_NUMPAD3, 	dirDOWNRIGHT	},
    {KEY_NUMPAD2, 	dirDOWN		}, {KEY_NUMPAD1, 	dirDOWNLEFT		},
    {KEY_NUMPAD4, 	dirLEFT		}, {KEY_NUMPAD7, 	dirUPLEFT		},
    {0,0}
};
/*****************************************************************************/
#ifdef _WINDOWS
int DelayTimes[5] = {1,20,30,50,60};//{0,12,24,32,60};//{0,12,24,40};
#else
int DelayTimes[5] = {0,96,140,280,560};//{0,32,96,320};
#endif
static BOOL s_characterBoostEnabled = TRUE;
static BOOL s_audioBoostEnabled = TRUE;
static BOOL s_larryMusicSpeedHackEnabled = TRUE;
static BOOL s_comboBoostEnabled = TRUE;

static U16 DelayTicks1024ToFrames(U32 ticks)
{
	U32 frames;

	frames = (ticks * 60 + 16383) / 16384;
	if (frames == 0) {
		frames = 1;
	}
	return (U16)frames;
}

static BOOL IsLarryMusicSpeedOverrideActive(void)
{
	if (!s_larryMusicSpeedHackEnabled) {
		return FALSE;
	}

	if ((strcmp(szGameID, "LLLLL") != 0) && (strcmp(szGameID, "LSL1") != 0)) {
		return FALSE;
	}

	return IsLSL1AmbientRoom(vars[vROOMNUM]);
}

static BOOL IsAudioBoostActive(void)
{
	if (!s_audioBoostEnabled) {
		return FALSE;
	}

	if (!TestFlag(fSOUND)) {
		return FALSE;
	}

	return IsCurrentPCMMusicLongerThanSeconds(4U);
}

static BOOL IsPoliceQuestDrivingRoomForMoveBoost(void)
{
	if (!(GameEnts && (strncmp(GameEnts->name, "Police Quest", 12) == 0))) {
		return FALSE;
	}

	return (BOOL)((vars[vROOMNUM] >= 10) && (vars[vROOMNUM] <= 25));
}

static BOOL HasMultipleMovingCharactersOnScreen(void)
{
	int moving_count;
	VOBJ *v;

	moving_count = 0;
	for (v = ViewObjs; v < &ViewObjs[MAX_VOBJ]; v++) {
		if ((v->flags & (oDRAWN | oANIMATE | oUPDATE)) != (oDRAWN | oANIMATE | oUPDATE)) {
			continue;
		}
		if ((v->direction == dirNONE) && (v->motion == mtNONE)) {
			continue;
		}
		moving_count++;
		if (moving_count > 1) {
			return TRUE;
		}
	}

	return FALSE;
}

static BOOL IsCharacterBoostActive(void)
{
	if (!s_characterBoostEnabled) {
		return FALSE;
	}

	if (IsPoliceQuestDrivingRoomForMoveBoost()) {
		return FALSE;
	}

	return HasMultipleMovingCharactersOnScreen();
}

static BOOL IsComboBoostActive(void)
{
	if (!s_comboBoostEnabled) {
		return FALSE;
	}

	return (BOOL)(IsAudioBoostActive() && IsLarryMusicSpeedOverrideActive());
}

static U8 GetEffectiveDelayIndex(void)
{
	U8 delay_index;

	delay_index = (U8)(vars[vDELAY] < 4 ? vars[vDELAY] : 4);
	if (delay_index == 2) {
		delay_index = 1;
	}

	return delay_index;
}

static U32 GetBaseGameplayDelayTicks1024(void)
{
	U8 delay_index;

	delay_index = GetEffectiveDelayIndex();
	return (U32)(DelayTimes[delay_index] * 10 + 1);
}

static U32 GetGameplayDelayTicks1024(void)
{
	U32 ticks;
	BOOL audio_boost_active;
	BOOL character_boost_active;

	ticks = GetBaseGameplayDelayTicks1024();
	audio_boost_active = IsAudioBoostActive();
	character_boost_active = IsCharacterBoostActive();

	/* Audio Boost, Character Boost, and Larry Room Boost can all stack their
	   own reductions when active. */
	if (audio_boost_active && (ticks > 1U)) {
		ticks = (ticks * 9U + 8U) >> 4;
	}
	if (character_boost_active && (ticks > 1U)) {
		ticks = (ticks * 3U + 2U) >> 2;
	}
	if (IsLarryMusicSpeedOverrideActive() && (ticks > 1U)) {
		ticks = (ticks * 11U + 8U) >> 4;
	}
	if (IsComboBoostActive() && (ticks > 1U)) {
		ticks = (ticks * 7U + 4U) >> 3;
	}

	return ticks;
}

static U16 GetGameplayDelayFrames(void)
{
	return DelayTicks1024ToFrames(GetGameplayDelayTicks1024());
}

void SetLarryMusicSpeedHackEnabled(BOOL enabled)
{
	s_larryMusicSpeedHackEnabled = enabled ? TRUE : FALSE;
}

BOOL IsLarryMusicSpeedHackEnabled(void)
{
	return s_larryMusicSpeedHackEnabled;
}

void SetComboBoostEnabled(BOOL enabled)
{
	s_comboBoostEnabled = enabled ? TRUE : FALSE;
}

BOOL IsComboBoostEnabled(void)
{
	return s_comboBoostEnabled;
}

void SetCharacterBoostEnabled(BOOL enabled)
{
	s_characterBoostEnabled = enabled ? TRUE : FALSE;
}

BOOL IsCharacterBoostEnabled(void)
{
	return s_characterBoostEnabled;
}

U8 GetActiveBoostMask(void)
{
	U8 mask;

	mask = 0;
	if (IsAudioBoostActive()) {
		mask |= 0x01U;
	}
	if (IsCharacterBoostActive()) {
		mask |= 0x02U;
	}
	if (IsLarryMusicSpeedOverrideActive()) {
		mask |= 0x04U;
	}
	if (IsComboBoostActive()) {
		mask |= 0x08U;
	}

	return mask;
}

U16 GetCurrentGameplaySpeedTenths(void)
{
	U32 base_ticks;
	U32 current_ticks;

	base_ticks = GetBaseGameplayDelayTicks1024();
	current_ticks = GetGameplayDelayTicks1024();
	if (current_ticks == 0U) {
		return 10U;
	}

	return (U16)((base_ticks * 10U + (current_ticks >> 1)) / current_ticks);
}

U16 GetCurrentGameplayDelayResultTenths(void)
{
#ifdef _WINDOWS
	return (U16)GetGameplayDelayTicks1024();
#else
	return (U16)((GetGameplayDelayTicks1024() * 600U + 8191U) / 16384U);
#endif
}

void SetAudioBoostEnabled(BOOL enabled)
{
	s_audioBoostEnabled = enabled ? TRUE : FALSE;
}

BOOL IsAudioBoostEnabled(void)
{
	return s_audioBoostEnabled;
}

void DoDelayNPoll()
{
	Delay(-1);
	PollInput();
}
/*****************************************************************************/
void Delay(int amt)
{
#ifdef _WINDOWS    
	if(amt==-1) {
		amt = (int)((GetGameplayDelayTicks1024() + 9U) / 10U);
	    while(amt>>4) {
    	    SystemUpdate();
    	    PollInput();
            Sleep(5);
    	    amt--;
        }
    } else {
        while(amt>0) {
            Sleep(5);
            amt--;
        }
    }
#else
	int delay;
	if(amt==-1) {
		delay = GetGameplayDelayFrames();
		while(delay--) {
			PollInput();
			WaitForFrames(1);
		}
    } else {
		delay = (64*amt)+1;
		WaitForFrames(DelayTicks1024ToFrames((U32)delay));
    }
#endif
}
/*****************************************************************************/
void PollInput()
{
	EVENT *event;
	CTLMAP *c;
	BOOL consumedHeldDirection = FALSE;

    while((BOOL)(event = ReadEvent()) && (!TestFlag(fPLAYERCOMMAND))) {
		if(event->type==EV_DIRECTION) {
        	vars[vEGODIR] =
                (IsWalkHoldActive() || (event->data != ViewObjs[0].direction))? event->data : 0;
            if(PLAYER_CONTROL)
				ViewObjs[0].motion = mtNONE;
			if(IsWalkHoldActive() && btnstate.state==BTN_HOLD)
				consumedHeldDirection = TRUE;
        } else {
        	U16 key = event->data;
            if(!GUI_ACTIVE){
                switch(event->data) {
                	case KEY_ENTER:
            			if(INPUT_ENABLED)
                			ExecuteInputDialog(TRUE);
                    	break;
                    case KEY_START:
                		ExecuteKeyboardDialog();
                        break;
                	case KEY_SELECT:
#if 0
                    	key = '\t'; // inventory
#else
                        if(!TEXT_MODE) {
#   ifndef FAKE_HERCULES
                            {{}} if (Y_ADJUST_CL == -CHAR_HEIGHT) Y_ADJUST_CL = 4;
                            else if (Y_ADJUST_CL <=            4) Y_ADJUST_CL = 32;
#   else
                            {{}} if (Y_ADJUST_CL == -CHAR_HEIGHT) Y_ADJUST_CL = 0;
                            else if (Y_ADJUST_CL <=            4) Y_ADJUST_CL = 8;
#endif
                            else                                  Y_ADJUST_CL = -CHAR_HEIGHT;
                            RedrawScreenAll();
                        }
                        key = '\0';
#endif
                    	break;
                	case KEY_TABREV:
                    	key = KEY_F3<<8; // repeat word
                    	break;
                	case KEY_TABFWD:
         				btnstate.btn = btnstate.kbkey;
         				btnstate.state = BTN_INJECTED;
                    	break;
                }

            }
			for(c = ctlMap; c < ctlMap+MAX_CONTROLLERS; c++)
				if(key == c->key) {
                	controllers[c->num]=1;
					break;
			}
        	vars[vKEYPRESSED] = (U8)key;
		}
		if(consumedHeldDirection)
			break;
	}
}
/*****************************************************************************/
EVENT *ReadEvent(void)
{
	KEY *k = (KEY*)keyDirs;
    int key=SystemCheckKey();
    if(key) {
		while(k->symbol && (k->symbol != key)) k++;
		if(k->symbol) {
			tmpEvent.type = EV_DIRECTION;
			tmpEvent.data = k->value;
		} else {
			tmpEvent.type = EV_ASCII;
			tmpEvent.data = key;
		}
    	return &tmpEvent;
    } else
    	if(IsWalkHoldActive() && btnstate.state==BTN_RELEASE)
			return &evStopEgo;
	return NULL;
}
/*****************************************************************************/
int PollKey()
{
	EVENT *event;
	if((BOOL)(event = ReadEvent()))
		return (event->type == EV_ASCII)? event->data : -1;
   	return 0;
}
/*****************************************************************************/
int CheckUserReply()
{
	switch(PollKey()) {
		case KEY_ENTER:
			return 1;
		case KEY_ESC:
			return 0;
	}
	return -1;
}
/*****************************************************************************/
int WaitForKey()
{
	int key;
    while((!(key = PollKey())) || (key == -1))
        Delay(5);
    return key;
}
/*****************************************************************************/
BOOL WaitEnterEsc()
{
	int key;
	while((key=CheckUserReply()) == -1) {
#ifdef _WINDOWS
		if(QUIT_FLAG) break;
#endif
		Delay(5);
	}
	return key;
}
/*****************************************************************************/
EVENT *WaitForEvent()
{
	EVENT *event;
	while(!(BOOL)(event = ReadEvent())
#ifdef _WINDOWS
                                       && !QUIT_FLAG
#endif
                                                    )
		Delay(5);
	return event;
}
/*****************************************************************************/

