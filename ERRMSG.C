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
#include <stdarg.h>
#include "gbagi.h"
#include "errmsg.h"
#include "system.h"
#include "text.h"
/******************************************************************************/
const char *Errors[TOTAL_ERRORS] = {
	"Bad AGI command code: $%02X",
	"Bad AGI test code: $%02X",
    "Bad AGI pic code: $%02X",
    "Bad view object o%d!",
    "Bad view object o%d! View not set!",
    "Bad view object o%d! Loop number %d, invalid!",
    "Bad view object o%d! Cel number %d, invalid!",
    "Can not set object view! View.%03d does not exist!",
    "Bad view object o%d! Cel not loaded!",
    "Message too verbose!",
    "Menu not set! Can not parse input!",
    "Menu not set! Can not add menu item: \"%s\"!",
    "Unable to locate menu item: %d!",
};
/******************************************************************************/
int ErrorMessage(int num, ...)
{
#ifdef _WINDOWS
	va_list argptr;
	int cnt;

	va_start(argptr, (char*)Errors[num]);
	cnt = vprintf((char*)Errors[num], argptr);
	va_end(argptr);
    AGIExit();

	return(cnt);
#else
   	//DrawStringAbs(0,16,(char*)Errors[num],0xEE);
	return 1;
#endif
}
/******************************************************************************/
void AGIExit()
{
	SystemExit();
}
/******************************************************************************/
