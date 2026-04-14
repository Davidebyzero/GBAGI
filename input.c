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
#include "menu.h"
#include "screen.h" // for Y_ADJUST_CL and RedrawScreenAll()
#ifdef _WINDOWS
#include <windows.h>
#endif
/*****************************************************************************/
EVENT tmpEvent, evStopEgo = {EV_DIRECTION, dirNONE};
BTNSTATE btnstate;
/*****************************************************************************/
static BOOL IsDispatcherCaption(const char *name)
{
	const char *n1 = "dispatcher";
	const char *n2 = "depatch";
	char c;
	int i, j;
	if(!name)
		return FALSE;
	for(i = 0; name[i]; i++) {
		j = 0;
		while(name[i + j] && n1[j]) {
			c = name[i + j];
			if(c >= 'A' && c <= 'Z') c |= 0x20;
			if(c != n1[j]) break;
			j++;
		}
		if(!n1[j]) return TRUE;
		j = 0;
		while(name[i + j] && n2[j]) {
			c = name[i + j];
			if(c >= 'A' && c <= 'Z') c |= 0x20;
			if(c != n2[j]) break;
			j++;
		}
		if(!n2[j]) return TRUE;
	}
	return FALSE;
}
/*****************************************************************************/
static BOOL IsDispatcherController(U8 ctl)
{
	MENU *m;
	MENUITEM *mi;
	for(m = menu; m; m = m->next) {
		for(mi = m->items; mi; mi = mi->next) {
			if(mi->controller == ctl && IsDispatcherCaption(mi->name))
				return TRUE;
		}
	}
	return FALSE;
}
/*****************************************************************************/
static BOOL HasDispatcherBinding(void)
{
	CTLMAP *c;
	for(c = ctlMap; c < ctlMap + MAX_CONTROLLERS; c++) {
		if(c->key && IsDispatcherController(c->num))
			return TRUE;
	}
	return FALSE;
}
/*****************************************************************************/
static BOOL IsLooseDispatcherHotkey(U16 key)
{
	/* Cover ctrl-d encodings plus plain 'd' seen in some Windows builds. */
	return (key == 4 || key == KEY_D || key == (KEY_D << 8) || key == 'd' || key == 'D');
}
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
		amt = (DelayTimes[vars[vDELAY]<4?vars[vDELAY]:4]);
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
     	delay = (DelayTimes[vars[vDELAY]<4?vars[vDELAY]:4])*10+1;
		while(REG_TM1CNT_L <= delay){
			PollInput();
        }
        REG_TM1CNT_H=0;
		REG_TM1CNT_L = 0;
		REG_TM1CNT_H = TIME_FREQUENcy1024 | TIME_ENABLE;
    } else {
		delay = (64*amt)+1;
		//Start the timer
		REG_TM0CNT_H = TIME_FREQUENcy1024 | TIME_ENABLE;
		REG_TM0CNT_L = 0;
		while(REG_TM0CNT_L <= delay){
			//PollInput();
        }
		REG_TM0CNT_H = 0;
    }
#endif
}
/*****************************************************************************/
void PollInput()
{
	EVENT *event;
	CTLMAP *c;

    while((BOOL)(event = ReadEvent()) && (!TestFlag(fPLAYERCOMMAND))) {
		if(event->type==EV_DIRECTION) {
        	vars[vEGODIR] =
                (WALK_HOLD || (event->data != ViewObjs[0].direction))? event->data : 0;
            if(PLAYER_CONTROL)
				ViewObjs[0].motion = mtNONE;
        } else {
        	U16 key = event->data;
			if(!GUI_ACTIVE && HasDispatcherBinding() && IsLooseDispatcherHotkey(key)) {
				char dispatchCmd[] = "Extender Depatch";
				ParseInput(dispatchCmd);
				continue;
			}
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
					if(!GUI_ACTIVE && IsDispatcherController(c->num)) {
						char dispatchCmd[] = "Extender Depatch";
						ParseInput(dispatchCmd);
						break;
					}
                 	controllers[c->num]=1;
					break;
				}
        	vars[vKEYPRESSED] = (U8)key;
		}
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
    	if(WALK_HOLD && btnstate.state==BTN_RELEASE)
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

