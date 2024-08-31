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
#include "status.h"
#include "text.h"
#include "screen.h"
#include "wingui.h"
/*****************************************************************************/
char tempStr[41];
/*****************************************************************************/
void WriteStatusLine()
{
	if(STATUS_VISIBLE) {      
//#ifdef FANCY_WINDOWS
//		ClearLine(statusRow, clWINDOW);
//#else
		ClearLine(statusRow, clWHITE);
//#endif
        sprintf(tempStr,"Score:%d of %d", vars[vSCORE], vars[vSCOREMAX]);
        DrawStringAbs(CHAR_WIDTH,statusRow*CHAR_HEIGHT,tempStr,clBLACK);
        sprintf(tempStr,"Sound:%s", TestFlag(fSOUND)?"on ":"off");
        DrawStringAbs(CHAR_WIDTH*30,statusRow*CHAR_HEIGHT,tempStr,clBLACK);
        
        //ShowPic();
	}
}
/*****************************************************************************/
