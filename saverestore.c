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
#include "saverestore.h"
#include "text.h"
#include "screen.h"
#include "wingui.h"
#include "logic.h"
#include "commands.h"
#include "views.h"
#include "picture.h"
#include "text.h"
#include "status.h"
#include "menu.h"
#include "input.h"
#include "errmsg.h"
#include "system.h"
#include "invobj.h"
/*****************************************************************************/

#ifdef _WINDOWS
	FILE *f;
#else
	U8 *pSaveMem;
#endif


#define MAX_SAVES 8
#define SAVE_FILE_SIZE 4096

const char szSaveHeader[]="GBAGI/Save_Game";
char szSaveName[MAX_SAVENAME_LEN+1],szAutoSave[MAX_SAVENAME_LEN+1];
char szSaveNames[MAX_SAVES][MAX_SAVENAME_LEN+1];
char szTemp1[16],szTemp2[128];
int saveSlot;
BOOL OK_CLOSE;

/*****************************************************************************/
S16 wnSaveRestoreProc(WND *w, U16 msg, U16 wParam, U32 lParam);

WND wnSaveRestore = {
	NULL,NULL,NULL,NULL,
	24,22,192,116,
	{0,0,0,0},{0,0,0,0},
	"S/R",
	0,
	0,
	wsRESIZABLE|wsTITLE|wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND bnSaveOK = {
	NULL,NULL,&wnSaveRestore,NULL,
	130,4,52,16,
	{0,0,0,0},{0,0,0,0},
	"Save",
	wnBUTTON,
	0,
	bsCAPTION|wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND bnSaveCancel = {
	NULL,NULL,&wnSaveRestore,NULL,
	130,24,52,16,
	{0,0,0,0},{0,0,0,0},
	"Cancel",
	wnBUTTON,
	0,
	bsCAPTION|wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND lbSaveFiles = {
	NULL,NULL,&wnSaveRestore,NULL,
	0,20,128,74,
	{0,0,0,0},{0,0,0,0},
	"",
	wnLISTBOX,
	0,
	wsSELECTABLE|wsEXTFIXED,
	(WNPROC)wnSaveRestoreProc
};
WND edSaveInput = {
	NULL,NULL,&wnSaveRestore,NULL,
	32,4,96,14,
	{0,0,0,0},{0,0,0,0},
	szSaveName,
	wnEDIT,
	0,
	wsSELECTABLE,
	(WNPROC)wnSaveRestoreProc
};
WND txSaveName = {
	NULL,NULL,&wnSaveRestore,NULL,
	2,8,30,8,
	{0,0,0,0},{0,0,0,0},
	"Name:",
	wnTEXT,
	0,
	0,
	(WNPROC)wnSaveRestoreProc
};
/*****************************************************************************/
S16 wnSaveRestoreProc(WND *w, U16 msg, U16 wParam, U32 lParam)
{
	switch(msg) {
    	case wmLISTBOX_CHANGE:
            edSaveInput.caption = ((LISTITEM*)lParam)->text;
            wDrawWnd(&edSaveInput, 0);
        	break;
    	case wmEDIT_CHANGE:
            wDrawWnd(&lbSaveFiles, 0);
        	break;
    	case wmBUTTON_CLICK:
        	if(w==&bnSaveOK) {
            	OK_CLOSE = TRUE;
             	WndDispose(&wnSaveRestore);
            } else if(w==&bnSaveCancel) {
            	OK_CLOSE = FALSE;
             	WndDispose(&wnSaveRestore);
            }
            break;
    }
	return 1;
}
/*****************************************************************************/
void InitSaveRestore()
{
    szAutoSave[0] = '\0';
}
/*****************************************************************************/
BOOL ExecuteSaveDialog(const char *szTitle, const char *szType)
{
	wnSaveRestore.caption = (char*)szTitle;
	bnSaveOK.caption = (char*)szType; 

    AddWindow(&wnSaveRestore);
    AddWindow(&bnSaveCancel);
    AddWindow(&bnSaveOK);
    AddWindow(&lbSaveFiles);
    edSaveInput.ext.edit.maxLen = 15;
    AddWindow(&edSaveInput);
    AddWindow(&txSaveName);

	WinGUIDoit();

    saveSlot = lbSaveFiles.ext.listbox.itemActive->index;

    return OK_CLOSE;
}
/*****************************************************************************/
int FillSaveList(BOOL RES,int maxsaves)
{
	int i=0;
	for(saveSlot=0;saveSlot<maxsaves;saveSlot++) {
    	SAVE_SEEK_SET(saveSlot*SAVE_FILE_SIZE);

    	FREADN(szTemp1,sizeof(szSaveHeader));
    	if(strcmp(szTemp1,szSaveHeader))
        	continue;
    	FREADN(szTemp1,sizeof(szGameID));
    	if(strcmp(szTemp1,szGameID))
        	continue;

    	FREAD(szSaveNames[saveSlot]);
        if(RES)ListBoxAdd(&lbSaveFiles,(char*)szSaveNames[saveSlot]);
        i++;
    }

    ListBoxSelect(&lbSaveFiles,0);

    return (i);
}
/*****************************************************************************/
void SRamMemCpy(U8 *a, U8 *b, int len)
{
 	while(len--)
    	*a++=*b++;
}
/*****************************************************************************/
BOOL SaveGame()
{
    int i, totalOverlays, totalPViews;

	OPEN_SAVE_FILE();

    if(szAutoSave[0]) {
     	strcpy(szSaveName,szAutoSave);
        saveSlot=0;
    } else {
    	ListBoxClear(&lbSaveFiles);
    	for(i=0;i<MAX_SAVES;i++) {
    		strcpy(szSaveNames[i],"-");
        	ListBoxAdd(&lbSaveFiles,(char*)szSaveNames[i]);
    	}
    	if(!FillSaveList(FALSE,MAX_SAVES)) {
    	}
    	
   		edSaveInput.style |= wsSELECTABLE;

		if(!ExecuteSaveDialog("Save Game", "Save")) {
        	CLOSE_SAVE_FILE();
        	return FALSE;
        }

        sprintf(szSaveName,szSaveNames[saveSlot]);
	}

	SAVE_SEEK_SET(saveSlot*SAVE_FILE_SIZE);

    FWRITE(szSaveHeader); // 16 bytes
    FWRITE(szGameID); 	// 8 bytes
    FWRITE(szSaveName);	// 24 bytes

    FWRITE(vars);
    FWRITE(flags);
    FWRITE(strings);

	FPUTB(PLAYER_CONTROL);
	FPUTB(TEXT_MODE);
	FPUTB(REFRESH_SCREEN);
	FPUTB(MENU_SET);
	FPUTB(INPUT_ENABLED);
	FPUTB(SOUND_ON);
	FPUTB(PIC_VISIBLE);
	FPUTB(PRI_VISIBLE);
	FPUTB(STATUS_VISIBLE);
	FPUTB(VOBJ_BLOCKING);
	FPUTB(WALK_HOLD);

	FPUTB(oldScore);
	FPUTB(horizon);
	FPUTB(picNum);
	FPUTB(minRow);
	FPUTB(inputPos);
	FPUTB(statusRow);
	FPUTB(textColour);
	FPUTB(textAttr);
	FPUTB(textRow);
	FPUTB(textCol);
	FPUTB(minRowY);
	FPUTB(ticks);
	FPUTB(cursorChar);
	FPUTB(IF_RESULT);

    FWRITE(&objBlock);
	FWRITE(ViewObjs);
	FWRITE(logScan);
    FWRITE(invObjRooms);

	FPUTW(msgX);
	FPUTW(msgY);
	FPUTW(msgHeight);
	FPUTW(msgWidth);
	FPUTW(maxWidth);
    FWRITE(&wndDraw);

	totalPViews	= (pPView-pViews);
    FPUTB( (U8)totalPViews );
    for(i=0;i<totalPViews;i++) {
     	FPUTB(pViews[i].view);
        FPUTB(pViews[i].loop);
        FPUTB(pViews[i].cel);
        FPUTB(pViews[i].x);
        FPUTB(pViews[i].y);
        FPUTB(pViews[i].pri);
        FPUTW(0); // for compatibility with old saves
    }
    totalOverlays = (pOverlay-overlays);
    FPUTB( (U8)totalOverlays );
    for(i=0;i<totalOverlays;i++) {
     	FPUTB(overlays[i]);
    }

    CLOSE_SAVE_FILE();

    return TRUE;
}
/*****************************************************************************/
BOOL RestoreGame()
{
	int temp4,i;
    int totalPViews, totalOverlays;
    VOBJ *v;

    OPEN_SAVE_FILE();
                    /*
    if(szAutoSave[0]) {
    	ListBoxClear(&lbSaveFiles);
   	 	if(!FillSaveList(TRUE,1)) {
			MessageBox("There are no previously saved games to restore.");
       		CLOSE_SAVE_FILE();
    		return FALSE;
   		}
    	SAVE_SEEK_SET(48);
    } else {           */
    	ListBoxClear(&lbSaveFiles);
   	 	if(!FillSaveList(TRUE,MAX_SAVES)) {
			MessageBox("There are no previously saved games to restore.");
       		CLOSE_SAVE_FILE();
    		return FALSE;
   		}
   		
   		edSaveInput.style &= ~wsSELECTABLE;
   		
		if(!ExecuteSaveDialog("Restore Game", "Restore")) {
       	 	CLOSE_SAVE_FILE();
    		return FALSE;
        }
		for(i=0;i<MAX_SAVES;i++) {
    		SAVE_SEEK_SET(i*SAVE_FILE_SIZE);

    		FREADN(szTemp1,sizeof(szSaveHeader));
   		 	if(strcmp(szTemp1,szSaveHeader))
	        	continue;
	    	FREADN(szTemp1,sizeof(szGameID));
	    	if(strcmp(szTemp1,szGameID))
	        	continue;

    		SAVE_SEEK_CUR(MAX_SAVENAME_LEN+1);

    	   	if(!saveSlot--) break;
	    }  /*
    }
    /*
    	SAVE_SEEK_SET(48);     */

	EraseBlitLists();   
	InitViewSystem();

    FREAD(vars);
    FREAD(flags);
    FREAD(strings);

	FGETB(PLAYER_CONTROL);
	FGETB(TEXT_MODE);
	FGETB(REFRESH_SCREEN);
	FGETB(MENU_SET);
	FGETB(INPUT_ENABLED);
	FGETB(SOUND_ON);
	FGETB(PIC_VISIBLE);
	FGETB(PRI_VISIBLE);
	FGETB(STATUS_VISIBLE);
	FGETB(VOBJ_BLOCKING);
	FGETB(WALK_HOLD);

	FGETB(oldScore);
	FGETB(horizon);
	FGETB(picNum);
	FGETB(minRow);
	FGETB(inputPos);
	FGETB(statusRow);
	FGETB(textColour);
	FGETB(textAttr);
	FGETB(textRow);
	FGETB(textCol);
	FGETB(minRowY);
	FGETB(ticks);
	FGETB(cursorChar);
	FGETB(IF_RESULT);

    FREAD(&objBlock);
	FREAD(ViewObjs);
	FREAD(logScan);
    FREAD(invObjRooms);

	FGETW(msgX);
	FGETW(msgY);
	FGETW(msgHeight);
	FGETW(msgWidth);
	FGETW(maxWidth);
    FREAD(&wndDraw);

    FGETB(totalPViews);
    for(i=0;i<totalPViews;i++) {
     	FGETB(pViews[i].view);
        FGETB(pViews[i].loop);
        FGETB(pViews[i].cel);
        FGETB(pViews[i].x);
        FGETB(pViews[i].y);
        FGETB(pViews[i].pri);
        FSKIPW(); // padded!
    }
    FGETB(totalOverlays);
    for(i=0;i<totalOverlays;i++) {
     	FGETB(overlays[i]);
    }
                       /*
    sprintf(szTemp1,"$%04X\n$%08X",(pSaveMem-GAMEPAK_RAM),pSaveMem);
	MessageBox(szTemp1); */

    CLOSE_SAVE_FILE();

	for(v=ViewObjs; v<&ViewObjs[MAX_VOBJ]; v++)
    	SetObjView(v,v->view);

    DrawPic(picNum);

	// 2/1/2004 fixed to point to start, OverlayPic/AddToPic will increment it accordingly
    pPView		= pViews;
    pOverlay	= overlays;

    for(i=0;i<totalOverlays;i++)
    	OverlayPic(overlays[i]);
    for(i=0;i<totalPViews;i++)
    	AddToPic(pViews[i].view, pViews[i].loop, pViews[i].cel, pViews[i].x, pViews[i].y, pViews[i].pri);

    PIC_VISIBLE = TRUE;
    ShowPic();

   	cCancelLine();
	WriteStatusLine();

    ClearControllers();

    SetFlag(fRESTORE);

	code = NULL;

    return TRUE;
}
/*****************************************************************************/
