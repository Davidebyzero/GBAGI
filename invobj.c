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
#include "invobj.h"
#include "gamedata.h"
#include "wingui.h"
#include "text.h"
/*****************************************************************************/

U8 invObjRooms[MAX_IOBJ];
/*****************************************************************************/
S16 wnInvProc(WND *w, U16 msg, U16 wParam, U32 lParam);

WND wnInv = {
	NULL,NULL,NULL,NULL,
	40,20,160,120,
	{0,0,0,0},{0,0,0,0},
	NULL,
	wnWINDOW,
	0,
	wsRESIZABLE|wsTITLE|wsSELECTABLE,
	(WNPROC)wnInvProc
};       
WND bnClose = {
	NULL,NULL,&wnInv,NULL,
	100,82,40,16,
	{0,0,0,0},{0,0,0,0},
	"Close",
	wnBUTTON,
	0,
	bsCAPTION|wsSELECTABLE,
	(WNPROC)wnInvProc
};
WND bnView = {
	NULL,NULL,&wnInv,NULL,
	16,82,80,16,
	{0,0,0,0},{0,0,0,0},
	"View Item",
	wnBUTTON,
	0,
	bsCAPTION|wsSELECTABLE,
	(WNPROC)wnInvProc
};
WND lbItems = {
	NULL,NULL,&wnInv,NULL,
	2,2,146,76,
	{0,0,0,0},{0,0,0,0},
	"Items",
	wnLISTBOX,
	0,
	wsSELECTABLE,
	(WNPROC)wnInvProc
};
WND sbItems;
/*****************************************************************************/
void InitObjSystem()
{
	memcpy(invObjRooms,(U8*)objRoomsStart,sizeof(invObjRooms));
}
/*****************************************************************************/
void ExecuteInvDialog()
{
	int i;

	for(i=0;i<256;i++)
    	if(invObjRooms[i]==0xFF)
    		break;
    if(i==256) {
    	/*wMessageBox(
        	"Inventory",
            "You are carrying: nothing!",
        	mbOK
        );      */

        MessageBox("Your inventory is empty.");
        return;
    }


    if(TestFlag(fINVSELECT)) {
    	wnInv.caption = "See Object"; 
    	bnClose.x = 100;
    	AddWindow(&wnInv);
    	AddWindow(&lbItems);
    	AddWindow(&bnClose);
    	AddWindow(&bnView);
    } else {
    	wnInv.caption = "Inventory";  
    	bnClose.x = 56;
    	AddWindow(&wnInv);
    	AddWindow(&lbItems);
    	AddWindow(&bnClose);
    }
	ListBoxSetScrollbar(&lbItems,&sbItems);

	for(i=0;i<256;i++)
    	if(objNames[i]&&invObjRooms[i]==0xFF)
			ListBoxAdd(&lbItems,(char*)objNames[i]);

	ListBoxSelect(&lbItems,0);

	WinGUIDoit();
}
/*****************************************************************************/
U8 FindObj(char *name)
{
	int i;
	for(i=0;i<255;i++)
    	if(objNames[i]==name)// no need for an strcmp!
        	return i;
    return 0xFF;
}
/*****************************************************************************/
S16 wnInvProc(WND *w, U16 msg, U16 wParam, U32 lParam)
{
	switch(msg) {
    	case wmBUTTON_CLICK:
        	if(wParam==KEY_START||w==&bnView) {
            	vars[vINVITEM] = FindObj(LIGetSelText(&lbItems));
             	WndDispose(&wnInv);
            } else if(wParam==KEY_SELECT||w==&bnClose) {
            	vars[vINVITEM] = 0xFF;
             	WndDispose(&wnInv);
            }
            break;
    }
	return TRUE;
} 
/*****************************************************************************/

