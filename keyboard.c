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
#include "wingui.h"
#include "parse.h"
#include "gamedata.h"
#include "system.h"
#include "screen.h"
#include "text.h"
#include "keyboard.h"
/*****************************************************************************/

/*****************************************************************************/
S16 wnKeysProc(WND *w, U16 msg, U16 wParam, U32 lParam);

WND wnKeys = {
	NULL,NULL,NULL,NULL,
	8,84,222,76,
	{0,0,0,0},{0,0,0,0},
	"Keyboard",
	0,
	0,
	wsRESIZABLE|wsTITLE|wsSELECTABLE,
	(WNPROC)wnKeysProc
};
WND kbKeyboard = {
	NULL,NULL,&wnKeys,NULL,
	0,0,212,56,
	{0,0,0,0},{0,0,0,0},
	"",
	wnKEYBOARD,
	0,
	wsSELECTABLE,
	(WNPROC)wnKeysProc
};
/*****************************************************************************/
void ExecuteKeyboardDialog()
{
	kbKeyboard.ext.keyboard.state 	= btnstate.kbstate;
	kbKeyboard.ext.keyboard.col 	= btnstate.kbcol;
	kbKeyboard.ext.keyboard.row	 	= btnstate.kbrow;
	kbKeyboard.ext.keyboard.selkey 	= btnstate.kbkey;

    AddWindow(&wnKeys);
    AddWindow(&kbKeyboard);

	WinGUIDoit();
}
/*****************************************************************************/
S16 wnKeysProc(WND *w, U16 msg, U16 wParam, U32 lParam)
{
	U8 *p;
	switch(msg) {
    	case wmBUTTON_CLICK:
        	switch(wParam) {
                case KEY_ESC:
             		WndDispose(&wnKeys);
                    break;
            }
            break;
    	case wmKEYBOARD_INPUT:
         	btnstate.btn = wParam;
         	btnstate.state = BTN_INJECTED;
			btnstate.kbstate=kbKeyboard.ext.keyboard.state;
			btnstate.kbcol=kbKeyboard.ext.keyboard.col;
			btnstate.kbrow=kbKeyboard.ext.keyboard.row;
			btnstate.kbkey=kbKeyboard.ext.keyboard.selkey;
            WndDispose(&wnKeys);
            break;
    }
	return TRUE;
}
/*****************************************************************************/

