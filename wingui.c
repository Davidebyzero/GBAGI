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
#include "gbagi.h"
#include "wingui.h"
#include "input.h"
#include "screen.h"
#include "text.h"
#include "system.h"

#include <math.h>

/******************************************************************************/
#define SZ_GUINAME "Desktop Advance v0.4"
/******************************************************************************/

S16 wnBaseProc(struct _WND *w, U16 msg, U16 wParam, void *pParam);

const WND wnBaseC = {
	NULL,NULL,NULL,NULL,
	0,0,SCREEN_WIDTH,SCREEN_HEIGHT,
	{0,0,SCREEN_MAXX,SCREEN_MAXY},{0,0,SCREEN_MAXX,SCREEN_MAXY},
	"",
	wnWINDOW,
	0,
	0,
	wnBaseProc
};

const DRAWPROC drawProcs[wnTOTAL] = {
	wDrawWindow,
	wDrawButton,
	wDrawText,
	wDrawEdit,
	wDrawListBox,
	wDrawScrollBar,
	wDrawKeyboard,
};

const MSGNODE msgProcs[wmMAX] = {
	{NULL,0},
	{wmActivate			, 0},
	{wmDeactivate		, 0},
	{wmShow				, 0},
	{wmHide				, 0},
	{wmPaint			, 0},
	{wmButtonPress		, mtPARENT},
	{wmButtonRelease	, mtPARENT},
	{wmButtonClick		, mtPARENT},
    {wmButtonHold		, mtPARENT},
    {wmListboxClick		, 0},
    {wmListboxChange	, 0},
    {wmEditChange		, 0},
    {wmKeyboardInput	, 0}
};

S16 wnMsgBoxProc(struct _WND *w, U16 msg, U16 wParam, void *pParam);

WND wnMsgBox = {
	NULL,NULL,NULL,NULL,0,0,0,0,{0,0,0,0},{0,0,0,0},NULL,
	wnWINDOW,
	0,
	wsSELECTABLE|wsBORDER|wsTITLE,
	wnMsgBoxProc
};
WND bnMsgOK = {
	NULL,NULL,&wnMsgBox,NULL,
    0,0,40,16,{0,0,0,0},{0,0,0,0},
    "OK",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	wnMsgBoxProc
};
WND bnMsgYes = {
	NULL,NULL,&wnMsgBox,NULL,
    0,0,40,16,{0,0,0,0},{0,0,0,0},
    "Yes",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	wnMsgBoxProc
};
WND bnMsgNo = {
	NULL,NULL,&wnMsgBox,NULL,
    0,0,40,16,{0,0,0,0},{0,0,0,0},
    "No",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	wnMsgBoxProc
};
WND bnMsgCancel = {
	NULL,NULL,&wnMsgBox,NULL,
    0,0,60,16,{0,0,0,0},{0,0,0,0},
    "Cancel",
	wnBUTTON,
	0,
	wsSELECTABLE|bsCAPTION,
	wnMsgBoxProc
};
U8 msgResult;

WND wnBase,*wnFirst;
WND *wnActive;
BOOL GUI_ACTIVE;
/******************************************************************************/

//------------
S16 wnProc(struct _WND *w, U16 msg, U16 wParam, void *pParam);
S16 wnProc2(struct _WND *w, U16 msg, U16 wParam, void *pParam);
S16 wnDesktopProc(struct _WND *w, U16 msg, U16 wParam, void *pParam);
WND wnDesktop,wnTest,wnTest1,wnTest2,bnBtn1,bnBtn2,bnBtn3,bnBtn4,sbScroll1,sbScroll2,txText1,txText2,txText3,txText4,txText5,txText6,txText7,edEdit1,lbListBox1,sbLbscroll;

const U8 pBmpData[]=
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x4C,0x4C,0x4C,0x01,0x00,0x01,0x01,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x4C,0x01,0x01,0x01,0x4C,0x01,0x4C,0x01,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x01,0x4C,0x4C,0x01,
	0x00,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x4C,0x4C,0x4C,0x01,
	0x01,0xEE,0xEE,0xEE,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x6E,0x7E,0x7E,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0x01,0x00,0x00,0x00,0x00,0x00,
	0x01,0x6E,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x01,0x00,0x00,0x00,0x00,0x00,
	0x01,0x6E,0x7E,0x7E,0x7E,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	0x01,0x6E,0x7E,0x6E,0x01,0x6E,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x01,0x00,
	0x01,0x6E,0x6E,0x01,0x6E,0x6E,0x6E,0x6E,0x6E,0x6E,0x6E,0x6E,0x6E,0x01,0x00,0x00,
	0x01,0x8E,0x01,0x6E,0x6E,0x6E,0x6E,0x6E,0x8E,0x8E,0x8E,0x8E,0x01,0x00,0x00,0x00,
	0x01,0x01,0x8E,0x8E,0x8E,0x8E,0x8E,0x8E,0x8E,0x8E,0x8E,0x01,0x00,0x00,0x00,0x00,
	0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const BMP bmp1 = {16,16,(U8*)pBmpData};
const U8 bmCounterBits[] = {
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
	0x01,0x31,0x31,0x31,0x31,0x31,0x31,0x01,
	0x31,0xB9,0xB9,0xB9,0xB9,0xB9,0xB9,0x31,
	0x31,0x01,0x01,0x01,0x01,0x01,0x01,0x31,
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
	0x31,0x01,0x01,0x01,0x01,0x01,0x01,0x31,
	0x31,0xB9,0xB9,0xB9,0xB9,0xB9,0xB9,0x31,
	0x01,0x31,0x31,0x31,0x31,0x31,0x31,0x01,
	0xF0,0x01,0x01,0x01,0x01,0x01,0x01,0xF0,
	0x00,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,
};
const BMP bmpCounter = {8,18,(U8*)bmCounterBits};


const char textSel[]=" ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const U8 bnColsSel[5]={0x9B,0x9B,0x13,0xFB,0x1B};
const U8 bnColsReg[5]={clWINDOW,cl3DLIGHTDK,cl3DSHADOW,clWINDOW,cl3DSHADOWDK};
const U8 inColsSel[5]={0x9B,0x13,0x9B,0x1B,0xFB};
const U8 inColsReg[5]={clWINDOW,clBLACK,clWHITE,0x78,0x77};
const int btnMsgWide[4] = {
	40,				// mbOK
	40+4+60,		// mbOKCANCEL
	40+4+40,		// mbYESNO
	40+4+40+4+60,	// mbYESNOCANCEL
};
/******************************************************************************/
void InitWinGUISystem(BOOL RESTART)
{
	if(!RESTART) {
		wnFirst		= &wnBase;
		memcpy(wnFirst,&wnBaseC,sizeof(WND));
	}
    wSetPort(&wnFirst->clRect);
    GUI_ACTIVE = FALSE;

    //-------------
}
/******************************************************************************/
char editBuf1[41];
void RunGUI1()
{                 /*
	WndCreate(&wnTest,	NULL,	wnWINDOW,	22, 25, 196, 120,	(wsBORDER|wsTITLE)|wsSELECTABLE,
		SZ_GUINAME,	wnProc);
 	WndCreate(&txText1,	&wnTest,wnTEXT,		8, 4, 0, 0,	txAUTOSIZE,
		"Welcome to GBAGI (BETA 0.80)!",NULL);
	WndCreate(&txText2,	&wnTest,wnTEXT,		33, 18, 0, 0,	txAUTOSIZE,
		"The Game Boy Advance",NULL);
	WndCreate(&txText3,	&wnTest,wnTEXT,		16, 28, 0, 0,	txAUTOSIZE,
		"Adventure Game Interpreter",NULL);
	WndCreate(&txText4,	&wnTest,wnTEXT,		34, 48, 0, 0,	txAUTOSIZE,
		"By Brian Provinciano",NULL);
	WndCreate(&txText5,	&wnTest,wnTEXT,		31, 58, 0, 0,	txAUTOSIZE,
		"http://www.bripro.com",NULL);
	WndCreate(&bnBtn1,	&wnTest,wnBUTTON,	63, 73, 60, 24,	bsCAPTION|wsSELECTABLE,
		"OK", wnProc);
    bnBtn1.style |= bsBITMAP;
    bnBtn1.ext.button.bitmap = &bmp1;


    WinGUIDoit();
                */
	WndCreate(&wnTest,	NULL,	wnWINDOW,	8, 12, 224, 128,	(wsBORDER|wsTITLE)|wsSELECTABLE,
		"GBAGI BETA *Not for public release*",	wnProc2);
 	WndCreate(&txText1,	&wnTest,wnTEXT,		4, 8, 0, 0,	txAUTOSIZE,
		"This demo shows off the capabilites",NULL);
	WndCreate(&txText2,	&wnTest,wnTEXT,		4, 18, 0, 0,	txAUTOSIZE,
		"of GBAGI, the Sierra adventure game",NULL);
	WndCreate(&txText3,	&wnTest,wnTEXT,		4, 28, 0, 0,	txAUTOSIZE,
		"emulator for GBA, with:"// Juha Terho's"
			,NULL);
    strcpy(editBuf1,"hello");
	WndCreate(&txText4,	&wnTest,wnEDIT,		5, 38, 200, 14,	wsSELECTABLE,
		editBuf1
		,NULL);
    txText4.ext.edit.maxLen = sizeof(editBuf1)-1;

	WndCreate(&txText5,	&wnTest,wnTEXT,		36, 54, 0, 0,	txAUTOSIZE,
		"Desktop Advance and GBAGI",NULL);
	WndCreate(&txText6,	&wnTest,wnTEXT,		57, 64, 0, 0,	txAUTOSIZE,
		"By Brian Provinciano",NULL);
	WndCreate(&txText7,	&wnTest,wnTEXT,		47, 74, 0, 0,	txAUTOSIZE,
		"http://www.bripro.com",NULL);
	WndCreate(&bnBtn1,	&wnTest,wnBUTTON,	88, 88, 40, 16,	bsCAPTION|wsSELECTABLE,
		"OK", wnProc2);

    WinGUIDoit();
}
/******************************************************************************/
void FreeWinGUISystem()
{
	// none of the windows should have been made by mallocing, so can't free 'em
    WndDispose(wnFirst);
	wnFirst		= NULL;
}
/******************************************************************************/
int WinGUIDoit()
{
	// would be so much easier with a mouse interface, oh well--GUIs are
	// for the users not the coders ;)
	// users wouldn't want to use a mouse cursor with the d-pad

	if(WndBringToFront(GetFirstWnd())==NULL) {
    	wSetPort(&wnFirst->clRect);
		return 0;
	}
	gfxGUIEnter();
    wSetPort(&wnActive->parent->clRect);

	RedrawAllWindows();

    do {
#ifdef _WINDOWS
		if(QUIT_FLAG) break;
#endif
		if(GBACheckButtons()->state==BTN_IDLE)
			continue;

		switch(btnstate.state) {
			case BTN_PRESS:
				switch(btnstate.btn) {
					// catch buttons
					/*case KEY_SELECT: // prev window
						WndBringToFront(WndSelectNext(wnActive));
						break;   */
					case KEY_TABREV: // prev window
						WndSelectPrev(wnActive->children);
						break;
					case KEY_TABFWD: // next window
						WndSelectNext(wnActive->children);
						break;
					// send to window
					//case KEY_ENTER:
                   default:
						WndMessage(GetActiveWnd(wnActive),wmBUTTON_PRESS,btnstate.btn,0,0);
				}
				break;
			case BTN_RELEASE:
				switch(btnstate.btn) {
					// catch buttons
					//case KEY_SELECT:
					case KEY_TABREV:
					case KEY_TABFWD:
						break;
					// send to window
				   //	case KEY_ENTER:
                   default:
						WndMessage(GetActiveWnd(wnActive),wmBUTTON_RELEASE,btnstate.btn,0,0);
				}
				break;
			case BTN_HOLD:
				switch(btnstate.btn) {
					// catch buttons
					//case KEY_SELECT:
					case KEY_TABREV:
					case KEY_TABFWD:
						break;
					// send to window
				   //	case KEY_ENTER:
                   default:
						WndMessage(GetActiveWnd(wnActive),wmBUTTON_HOLD,btnstate.btn,0,0);
				}
				break;
		}
    } while(wnActive);

    wSetPort(&wnFirst->clRect);
	gfxGUIExit();

    return 0;
}
/******************************************************************************/
void wSetPort(_RECT *r)
{
    memcpy(&port,r,sizeof(_RECT));
}
/******************************************************************************/
void wGetPort(_RECT *r)
{
    memcpy(r,&port,sizeof(_RECT));
}
/******************************************************************************/
BOOL WndCreate(WND *w, WND *parent, U8 type, S16 x, S16 y, S16 width, S16 height, U16 style, char *caption, WNPROC proc)
{
	memset(w,0,sizeof(WND));
 	w->parent	= parent;
 	w->type		= type;
 	w->x		= x;
 	w->y		= y;
 	w->width	= width;
 	w->height	= height;
 	w->style	= style;
 	w->caption	= caption;
 	w->proc		= proc;

    return AddWindow(w);
}
/******************************************************************************/
BOOL AddWindow(WND *w)
{
	WND *wnLast=NULL;

    if(w->type>=wnTOTAL) return FALSE;

	if(!w->parent)
		w->parent = wnFirst;
                           
    w->state	&= ~wtNOUPDATE;
    
    if(w->parent->state&wtNOPAINT)
    	w->state|=wtNOPAINT;
    else
    	w->state&=~wtNOPAINT;

	if( w->parent->children && ((wnLast = GetWndLast(w->parent))!=NULL) )
		wnLast->next = w;
	else
		w->parent->children = w;
	w->prev		= wnLast;
	w->next		= NULL;

	CalcWndRect(w);

    if(!WndSelect(w,drBACKWARD));
    	wDrawWnd(w,wdFULL);

	return TRUE;
}
/******************************************************************************/
void CalcWndRect(WND *w)
{
	_RECT abRect, *pClRect = &w->parent->clRect;

    switch(w->type) {
		case wnTEXT:
    		if(w->style&txAUTOSIZE) {
            	if(w->caption) {
               		w->width	= strlen(w->caption)*CHAR_WIDTH;
               		w->height	= CHAR_HEIGHT;
                } else {
                	w->width	= 0;
                	w->height	= 0;
                }
        	}
            if(w->style&wsSELECTABLE) {
             	w->width	+= wmxTEXTBORDER<<1;
             	w->height	+= wmxTEXTBORDER<<1;
            }
        	break;
   		default:;
    }
    w->x &= ~1;
    w->y &= ~1;
    w->width = (w->width+1)&(~1);
    w->height = (w->height+1)&(~1);

    w->rect.left	= w->x;
    w->rect.top		= w->y;
    w->rect.right	= w->x+w->width-1;
    w->rect.bottom	= w->y+w->height-1;

	abRect.left		= pClRect->left	+	w->x;
	abRect.top 		= pClRect->top	+	w->y;
	abRect.right 	= abRect.left	+	w->width-1;
	abRect.bottom 	= abRect.top	+	w->height-1;

    switch(w->type) {
    case wnWINDOW:
    	if(w->style&wsRESIZABLE) {
			abRect.left		+= wmxBORDERRESIZE;
			abRect.top 		+= wmxBORDERRESIZE;
			abRect.right 	-= wmxBORDERRESIZE;
			abRect.bottom 	-= wmxBORDERRESIZE;
        } else {
    		if(w->style&wsBORDER) {
				abRect.left		+= wmxBORDERFIXED;
				abRect.top 		+= wmxBORDERFIXED;
				abRect.right 	-= wmxBORDERFIXED;
				abRect.bottom 	-= wmxBORDERFIXED;
        	}
        }
    	if(w->style&wsTITLE) {
        	abRect.top 		+= wmxTITLEHEIGHT;
        }
        break;
	case wnBUTTON:
    	abRect.left		+= wmxBUTTONBORDER;
        abRect.top 		+= wmxBUTTONBORDER;
        abRect.right 	-= wmxBUTTONBORDER;
        abRect.bottom 	-= wmxBUTTONBORDER;
        break;
	case wnEDIT:
    	abRect.left		+= wmxINSETBORDER;
        abRect.top 		+= wmxINSETBORDER;
        abRect.right 	-= wmxINSETBORDER;
        abRect.bottom 	-= wmxINSETBORDER;
        break;
	case wnSCROLLBAR:
    	if(w->style&sbBORDER) {
    		abRect.left		+= wmxSCROLLBORDER;
        	abRect.top 		+= wmxSCROLLBORDER;
        	abRect.right 	-= wmxSCROLLBORDER;
        	abRect.bottom 	-= wmxSCROLLBORDER;
        }
        break;
	case wnLISTBOX:
    	abRect.left		+= wmxINSETBORDER;
        abRect.top 		+= wmxINSETBORDER;
        abRect.right 	-= wmxINSETBORDER;
        //if(w->style&lbVSCROLL)
        //	abRect.right 	-= wmxSCROLLBARWIDTH;
        abRect.bottom 	-= wmxINSETBORDER;
        break;
	case wnTEXT:
    	if(w->style&wsSELECTABLE) {
    		abRect.left		+= wmxTEXTBORDER;
        	abRect.top 		+= wmxTEXTBORDER;
        	abRect.right 	-= wmxTEXTBORDER;
        	abRect.bottom 	-= wmxTEXTBORDER;
        }
        break;
   		default:;
    }

	if(abRect.left<0)
		abRect.left		= 0;
	if(abRect.top<0)
		abRect.top		= 0;
	if(abRect.right>pClRect->right)
		abRect.right	= pClRect->right;
	if(abRect.bottom>pClRect->bottom)
		abRect.bottom	= pClRect->bottom;

    memcpy(&w->clRect, &abRect,sizeof(_RECT));
}
/******************************************************************************/
BOOL IsOnlyChild(WND *w)
{
	return( !w->parent || (w->prev==NULL && w->next==NULL) );
}
/******************************************************************************/
WND *WndBringToFront(WND *wNewActive)
{                     /*
    WND *oldActive;
	if(wNewActive&&(oldActive=wNewActive->parent->children)!=wNewActive) {

    	if(wNewActive->prev)
    		wNewActive->prev->next = wNewActive->next;
    	if(wNewActive->next)
    		wNewActive->next->prev = wNewActive->prev;

    	wNewActive->parent->children = wNewActive;
    	wNewActive->prev = NULL;
    	wNewActive->next = oldActive;
		if(oldActive)
    		oldActive->prev = wNewActive;
 	}
    if((wNewActive&&wNewActive->type==wnWINDOW)||!wnActive||oldActive==wnActive)
    	*/wnActive = wNewActive;
    return wNewActive;
}
/******************************************************************************/
WND *GetFirstWnd()
{
	WND *w=wnFirst->children;
	while(w) {
    	if(w->style&wsSELECTABLE&&(!(w->style&wsHIDDEN))&&(!(w->style&wsDISABLED))) {
            if(w!=wnFirst->children)
        		while(WndSelectNext(wnFirst->children)!=w);
        	break;
        }
        w = w->next;
    }
	return w;
}
/******************************************************************************/
WND *GetActiveWnd(WND *wfirst)
{
	WND *w=wfirst,*wnext=w;
	while(w) {
        if(!wnext) return w;
        wnext = w->children;
        while(wnext) {
        	if(wnext->style&wsSELECTABLE&&(!(wnext->style&wsHIDDEN))&&(!(wnext->style&wsDISABLED))) {
        		w=wnext;
                break;
            }
            wnext = wnext->next;
        }
	}
	return w;
}
/******************************************************************************/
WND *GetWndLast(WND *parent)
{
	WND *w;
    if(!parent) return NULL;
	w=parent->children;
	while(w) {
		if(!w->next)
			break;
		w=w->next;
	}
	return w;
}
/******************************************************************************/
WND *GetWndLastSel(WND *wStart)
{
	WND *w,*ws=NULL;
    if(!wStart) return NULL;
    w=wStart->next;
	while(w) {
    	if((w->style&(wsSELECTABLE|wsHIDDEN|wsDISABLED))==wsSELECTABLE)
        	ws=w;
		w=w->next;
	}
	return ws?ws:GetWndFirstSel(wStart);
}
/******************************************************************************/
BOOL WndIsActive(WND *w)
{
	WND *m;
    if(w&&w->parent) {
		m=w->parent->children;
    	while(m) {
    	if((w->style&(wsSELECTABLE|wsHIDDEN|wsDISABLED))==wsSELECTABLE)
    	    	return (m==w);
            m=m->next;
    	}
    }
    return FALSE;
}
/******************************************************************************/
WND *GetWndFirstSel(WND *wEnd)
{
	WND *w,*ws=NULL;
    if(!wEnd||!wEnd->parent) return NULL;
    w=wEnd->parent->children;
	while(w&&w!=wEnd) {
    	if((w->style&(wsSELECTABLE|wsHIDDEN|wsDISABLED))==wsSELECTABLE)
        	ws=w;
		w=w->next;
	}
	return ws;
}
/******************************************************************************/
WND *GetWndPrevSel(WND *wStart)
{
	WND *w,*ws=NULL;
    if(!wStart||!wStart->parent) return NULL;
    w=wStart->parent->children;
	while(w&&w!=wStart) {
    	if((w->style&(wsSELECTABLE|wsHIDDEN|wsDISABLED))==wsSELECTABLE)
        	ws=w;
        w=w->next;
	}
	return ws?ws:GetWndLastSel(wStart);
}
/******************************************************************************/
WND *GetWndNextSel(WND *wStart)
{
	WND *w=wStart;
	while(w) {
		if(!(BOOL)(w=w->next))
			break;
    	if((w->style&(wsSELECTABLE|wsHIDDEN|wsDISABLED))==wsSELECTABLE)
        	return w;
	}
	return GetWndFirstSel(wStart);
}
/******************************************************************************/
WND *WndFindPrev(WND *w)
{
	if(IsOnlyChild(w))
    	return NULL;
	if(w->prev)
    	return w->prev;
	return GetWndLast(w->parent);
}
/******************************************************************************/
WND *WndFindNext(WND *w)
{
	if(IsOnlyChild(w))
    	return NULL;
	if(w->next)
    	return w->next;
	return w->parent->children;
}
/******************************************************************************/
WND *WndSelect(WND *wSelect, int direction)
{
	WND *wLast,*wFirst,*wNext;
    if(!wSelect || !(wSelect->style&wsSELECTABLE) || (!wSelect->next && !wSelect->prev) ||
      !(BOOL)(wFirst = wSelect->parent->children) || (wFirst==NULL) || (wFirst==wSelect) ||
      !(BOOL)(wLast=GetWndLast(wSelect->parent)) || (wLast==wFirst))
    	return NULL;
    if(WndMessage(wFirst,wmDEACTIVATE,0,0,0)) {
        if(wLast->prev==wFirst) {
        	wLast->prev = NULL;
            wLast->next = wFirst; 
            wFirst->prev = wLast;
            wFirst->next = NULL;
        	wSelect->parent->children = wLast;
        } else {
    	if(wSelect->prev)
        	wSelect->prev->next	= wSelect->next;
        if(wSelect->next)
        	wSelect->next->prev	= wSelect->prev;
        if(direction==drFORWARD) {
        	if(wSelect == wLast) {
             	wFirst->prev = wSelect;
                wSelect->next = wFirst;
                if(wSelect->prev)
                	wSelect->prev->next = NULL;
                wSelect->prev = NULL;
            } else if(wSelect == wFirst) {
             	wLast->next = wSelect;
                wSelect->prev = wLast;
                if(wSelect->next)
                	wSelect->next->prev = NULL;
                wSelect->prev = NULL;
            } else {
           		wLast->next = wFirst;
            	wFirst->prev = wLast;
      			wNext = wFirst->next;
            	wFirst->next = NULL;
            	wSelect->next = wNext;
            	if(wNext)
            		wNext->prev = wSelect;
            	wSelect->prev = NULL;
            }
                       /*
            if(wFirst->next)
            	wFirst->next->prev	= wSelect;
        	wSelect->next		= wFirst->next;

        	wLast->next			= wFirst;
        	wFirst->prev		= wLast;
        	wFirst->next		= NULL;
        	wSelect->prev		= NULL; */
        } else {
            wSelect->next = wFirst;
            wSelect->prev = NULL;
            wFirst->prev = wSelect;
        }
        wSelect->parent->children= wSelect;
        }

    	wDrawWnd(wFirst,wdFULL);
    	wDrawWnd(wSelect,wdFULL);
     	WndMessage(wSelect,wmACTIVATE,0,0,0);
    }
    return wSelect;
}
/******************************************************************************/
WND *WndSelectPrev(WND *wStart)
{
	WND *w;
	if(!wStart||!(BOOL)(w=GetWndPrevSel(wStart))||w==wStart) return NULL;
	return WndSelect(w,drBACKWARD);
}
/******************************************************************************/
WND *WndSelectNext(WND *wStart)
{
	WND *w;
	if(!wStart||!(BOOL)(w=GetWndNextSel(wStart))||w==wStart) return NULL;
	return WndSelect(w,drFORWARD);
}
/******************************************************************************/
LISTITEM *LIMalloc()
{       /*
	LISTITEM *li=libuf;
    while((li<LIBUF_END)&&(li->index&lfINITIALIZED))
    	li++;
    if(li==LIBUF_END)
    	return NULL;
    li->index=lfINITIALIZED;
    return li;  */
    return (LISTITEM*)malloc(sizeof(LISTITEM));
}
/******************************************************************************/
void LIFree(LISTITEM *li)
{
    //memset(li,0,sizeof(LISTITEM));
    if(li) free(li);
}
/******************************************************************************/
void ListBoxClear(WND *w)
{
	LISTITEM *l=w->ext.listbox.itemFirst;
    LISTITEM *next;
    while(l) {   
        next=l->next;
     	if(!(l->index&0x8000))
        	free(l);
        l=next;
    }
    memset(&w->ext,0,sizeof(w->ext));
}
/******************************************************************************/
S16 ListBoxAdd(WND *w, char *text)
{
	LISTITEM *l;
    int ti,ai,itemheight=GETLBMAXVISIBLE(w);
    if(!(BOOL)(l=LIMalloc())) return -1;

	if(!w->ext.listbox.itemFirst) {
     	w->ext.listbox.itemFirst = l;
    } else if(w->ext.listbox.itemLast) {
        w->ext.listbox.itemLast->next = l;
    }               
    ai = w->ext.listbox.itemCount;
    l->prev = w->ext.listbox.itemLast;
    l->next = NULL;
    l->text = text;
    l->index = ai;
    w->ext.listbox.itemLast = l;
    w->ext.listbox.itemActive = l;

    if((ti = LIFindIndex(w,w->ext.listbox.itemTop))==-1)
    	ti=0;
    if(ti<=(ai-itemheight))
    	ti=ai-itemheight+1;
    w->ext.listbox.itemTop = LIFind(w,ti);

    w->ext.listbox.itemCount++;

    wDrawWnd(w,0);

    return ai;
}
/******************************************************************************/
BOOL ListBoxDel(WND *w, int index)
{
	LISTITEM *l;
    if(!(BOOL)(l=LIFind(w, index))) return FALSE;

    if(l->prev)
    	l->prev->next = l->next;
    if(l->next)
    	l->next->prev = l->prev;
	if(l==w->ext.listbox.itemFirst)
     	w->ext.listbox.itemFirst = l->next;
    if(l==w->ext.listbox.itemLast)
        w->ext.listbox.itemLast = l->prev;
	LIFree(l);
	w->ext.listbox.itemCount--;

    wDrawWnd(w,0);

    return TRUE;
}
/******************************************************************************/
LISTITEM *LIFind(WND *w, int index)
{
	S16 i;
	LISTITEM *l=w->ext.listbox.itemFirst;

    for(i=0;i<index;i++) {
    	if(!l->next)
        	break;
        l=l->next;
    }
    return l;
}
/******************************************************************************/
S16 LIFindIndex(WND *w, LISTITEM *li)
{           /*
	S16 i=0;
	LISTITEM *l=w->ext.listbox.itemFirst;
    while(l) {
     	if(l==li)
        	return i;
        i++;
        l=l->next;
    }     */
    if(li)
    	return li->index&0x7FFF;
    return -1;
}
/******************************************************************************/
char *LIGetSelText(WND *w)
{
	LISTITEM *l;
    if(w&&(l=w->ext.listbox.itemActive)!=NULL)
    	return l->text;
    return "";
}
/******************************************************************************/
void ListBoxSelect(WND *w,int index)
{
	w->ext.listbox.itemActive=LIFind(w,index);
    ListBoxSelItem(w,0);
}    
/******************************************************************************/
void ListBoxSelItem(WND *w,int diff)
{
	LISTITEM *liNew;
    int lidex,todex,itemheight=(GETCLIENTHEIGHT(w)-CHAR_HEIGHT/2)/CHAR_HEIGHT;
	if(!w->ext.listbox.itemCount||!w->ext.listbox.itemActive||!w->ext.listbox.itemFirst||!w->ext.listbox.itemLast)
    	return;
    lidex=LIFindIndex(w,w->ext.listbox.itemActive)+diff;
    todex=LIFindIndex(w,w->ext.listbox.itemTop);
	if((liNew=LIFind(w,lidex))!=NULL) {
    	w->ext.listbox.itemActive = liNew;
        if(todex>lidex)
    		w->ext.listbox.itemTop = liNew;
        else if(todex+itemheight<lidex)
    		w->ext.listbox.itemTop = LIFind(w,lidex-itemheight);
		wDrawWnd(w,0);
     	WndMessage(w, wmLISTBOX_CHANGE, lidex, liNew, 0);
    }
}
/******************************************************************************/
void ListBoxSelItemCh(WND *w,int diff)
{
	LISTITEM *liNew;
    U8 ch=w->ext.listbox.itemActive->text[0];
    int lidex,todex,itemheight=GETLBMAXVISIBLE(w)-1;
	if(!w->ext.listbox.itemCount||!w->ext.listbox.itemActive||!w->ext.listbox.itemFirst||!w->ext.listbox.itemLast)
    	return;
    lidex=LIFindIndex(w,w->ext.listbox.itemActive);
    if(diff==-1) {
    	liNew=w->ext.listbox.itemActive->prev;
    	while(liNew) {
     		if(liNew->text[0]<ch)
        		break;
            liNew=liNew->prev;
    	}
    } else {
    	liNew=w->ext.listbox.itemActive->next;
    	while(liNew) {
     		if(liNew->text[0]>ch)
        		break;
            liNew=liNew->next;
    	}
    }
    if(liNew) {
    	todex=LIFindIndex(w,w->ext.listbox.itemTop);
    	lidex=LIFindIndex(w,liNew);
    	w->ext.listbox.itemActive = liNew;
        if(todex>lidex)
    		w->ext.listbox.itemTop = liNew;
        else if(todex+itemheight<lidex)
    		w->ext.listbox.itemTop = LIFind(w,lidex-itemheight);
		wDrawWnd(w,0);      
     	WndMessage(w, wmLISTBOX_CHANGE, lidex, liNew, 0);
    }
}
/******************************************************************************/
void ListBoxPrevItem(WND *w)
{
	ListBoxSelItem(w,-1);
}
/******************************************************************************/
void ListBoxNextItem(WND *w)
{
	ListBoxSelItem(w,+1);
}
/******************************************************************************/
void ListBoxPrevItemCh(WND *w)
{
	ListBoxSelItemCh(w,-1);
}
/******************************************************************************/
void ListBoxNextItemCh(WND *w)
{
	ListBoxSelItemCh(w,+1);
}
/******************************************************************************/
void ListBoxSetScrollbar(WND *lb,WND *sb)
{
	if(!lb||!sb) return;
    lb->ext.listbox.scrollbar = sb;
    sb->parent = lb;
    sb->y = 0;
    sb->x = (GETCLIENTWIDTH(lb)-wmxSCROLLBARWIDTH);
    sb->width = wmxSCROLLBARWIDTH;
    sb->height = GETCLIENTHEIGHT(lb);
    sb->type = wnSCROLLBAR;
    sb->state = 0;
    sb->style = 0;
	AddWindow(sb);
}
/******************************************************************************/
int FindTextSelChar(char cA)
{
 	int c;
    if(cA>='a'&&cA<='z')
    	cA&=0xDF;
    for(c=sizeof(textSel)-2;c>=0;c--) {
     	if(textSel[c]==cA) return c;
    }
    return sizeof(textSel)-2;
}
/******************************************************************************/
void EditScrollChar(WND *w,int dir)
{
	int col = w->ext.edit.col;
    int c = FindTextSelChar(w->caption[col]);
    int min = w->style&esDIGITONLY?27:0;
	if(c!=-1) {
    	c += dir;
        if(c<min)
        	c = sizeof(textSel)-2;
        else if(c>sizeof(textSel)-2)
        	c=min;
        w->caption[col] = textSel[c];
		wDrawWnd(w,wdFULL);
		WndMessage(w, wmEDIT_CHANGE, 0, 0, 0);

#ifdef _WINDOWS
    	Delay(20);
#else
    	Delay(15);
#endif
    }
}
/******************************************************************************/
void EditScrollCol(WND *w,int dir, BOOL del)
{
	int col=w->ext.edit.col;
    if(col&&del)
     	w->caption[col]		= '\0';
    if(!del && ((w->style&esDIGITONLY)&&(w->caption[col]<'0'||w->caption[col]>'9'))) {
     	w->caption[col]		= '0';
        w->caption[col+1]	= '\0';
    }
    col+=dir;
    if(col<0) col=0;
    else if(col>=w->ext.edit.maxLen)
    	col = w->ext.edit.maxLen-1;
    if(!w->caption[col]) {
      	w->caption[col]		= (w->style&esDIGITONLY)?'0':' ';
        w->caption[col+1]	= '\0';
    }

    w->ext.edit.col = col;
    wDrawWnd(w,wdFULL);

	if(del)
    	WndMessage(w, wmEDIT_CHANGE, 0, 0, 0);

#ifdef _WINDOWS
    	Delay(40);
#else
    	Delay(20);
#endif
}
/******************************************************************************/
S16 ExecuteWndProc(WND *w, U32 msg, U16 wParam, void *pParam)
{
 	if(w && w->proc && !(w->state&wtNOUPDATE))
    	return w->proc(w, (U16)msg, wParam, pParam);
    return 0;
}
/******************************************************************************/
S16 WndMessage(WND *w, U32 msg, U16 wParam, void *pParam, U16 flags)
{
	S16 ret=1;
	if(msg==0||msg >= wmMAX||!w) return 0;

    if(msgProcs[msg].proc(w,wParam,pParam,flags))
    	ret = ExecuteWndProc(w, msg, wParam, pParam);

    if(msgProcs[msg].flags&mtPARENT) {
    	while((w = w->parent)!=NULL) {
			if(msgProcs[msg].proc(w,wParam,pParam, mtPARENT))
				ExecuteWndProc(w, msg, wParam, pParam);

    	}
    }
	return ret;
}
/******************************************************************************/
void gHLine(int x1, int x2, int y, U8 colour)
{
	while(x1<=x2)
    	PlotPix(x1++,y,colour);
}
/******************************************************************************/
void gVLine(int y1, int y2, int x, U8 colour)
{
	while(y1<=y2)
    	PlotPix(x,y1++,colour);
}
/******************************************************************************/
void gRect(int x1, int y1, int x2, int y2, U8 colour)
{            /*
	int x;
	while(y1<=y2) {
		for(x=x1;x<=x2;x++)
        	PlotPix(x,y1,colour);
        y1++;
    }     */
    RectFillX(x1, y1, x2, y2, colour);
}
/******************************************************************************/
void gWndRect(int x1, int y1, int x2, int y2)
{
	int x;
	while(y1<=y2) {
        if(y1&1)
			for(x=x1;x<=x2;x++)
        		ShadowPix(x,y1);
        else
			for(x=x1;x<=x2;x++)
        		HilightPix(x,y1);
        y1++;
    }
}
/******************************************************************************/
void gDrawShadow(int x1, int y1, int x2, int y2)
{          /*
	int x,y,yy;
    y=y1+2;
    yy=y2-3;
	while(y<=yy) {
    	for(x=x2-3;x<=x2;x++)
        	ShadowPix(x,y);
        y++;
    }
    y=y1+2;
    yy=y2-1;
	while(y<=yy) {
    	for(x=x1+2;x<=x2;x++)
        	ShadowPix(x,y);
        y++;
    }
    x2--;
    for(x=x1+2;x<=x2;x++)
    	ShadowPix(x,y);  */
    ShadowBoxX(x2-2,y1,x2,y2);
    ShadowBoxX(x1,y2-2,x2,y2);
}
/******************************************************************************/
void gDrawShadowSolid(int x1, int y1, int x2, int y2, U8 col)
{
	int x,y,yy;
    y=y1+2;
    yy=y2-3;
	while(y<=yy) {
    	for(x=x2-3;x<=x2;x++)
        	PlotPix(x,y,col);
        y++;
    }
    y=y1+2;
    yy=y2-1;
	while(y<=yy) {
    	for(x=x1+2;x<=x2;x++)
        	PlotPix(x,y,col);
        y++;
    }
    x2--;
    for(x=x1+2;x<=x2;x++)
    	PlotPix(x,y,col);
}
/******************************************************************************/
void gOutline(int x1, int y1, int x2, int y2, U8 hlgt, U8 shad)
{
	gHLine(x1, x2, y1, hlgt);
	gVLine(y1, y2, x1, hlgt);
	gHLine(x1, x2, y2, shad);
	gVLine(y1, y2, x2, shad);
}
/******************************************************************************/
void gROutline(int x1, int y1, int x2, int y2, U8 hlgt, U8 shad)
{
	gHLine(x1+1, x2-1, y1, hlgt);
	gVLine(y1+1, y2-1, x1, hlgt);
	gHLine(x1+1, x2-1, y2, shad);
	gVLine(y1+1, y2-1, x2, shad);
}
/******************************************************************************/
void gDrawWindowFrame(WND *w)
{
	int x1=w->rect.left, y1=w->rect.top, x2=w->rect.right, y2=w->rect.bottom;
    BOOL WND_ACTIVE = WndIsActive(w);
    gDrawShadow(  x1+2,  y1+2,  x2,  y2  );
    x2-=2;
    y2-=2;
	if((w->style&wsBORDER)||(w->style&wsRESIZABLE)) {
		gROutline(  x1,  y1,  x2,  y2, clBORDER,  clBORDER);
		gOutline(++x1,++y1,--x2,--y2, cl3DLIGHT, cl3DSHADOW);
		if(w->style&wsRESIZABLE) {
			gOutline(++x1,++y1,--x2,--y2, cl3DLIGHTDK, cl3DSHADOWDK);
			gOutline(++x1,++y1,--x2,--y2, clWINDOW, clWINDOW);
    	}
        if(w->style&wsTITLE) {
        	y1++;
    		gRect(++x1,y1,--x2,y1+wmxTITLEHEIGHT, WND_ACTIVE?clTITLEACTIVE:clTITLEINACTIVE);
           	DrawStringAbs(x1+4,y1+2,w->caption,clTITLETEXT);
        	gRect(x1,y1+wmxTITLEHEIGHT,x2,--y2, clWINDOW);
        } else
        	gRect(++x1,++y1,--x2,--y2, clWINDOW);
    } else
    	gRect(x1,y1,x2,y2, clWINDOW);
}
/******************************************************************************/
void gDrawButtonFrame(int x1,int y1,int x2,int y2,U16 flags)
{
    U8 *bnCols=(flags&dbfSELECTED)?(U8*)bnColsSel:(U8*)bnColsReg;
    if(flags&dbfDOWN) {
		gRect(x1+2,y1+2,x2-2,y2-2, 0xB1);
    	gOutline(x1,y1,  x2,  y2, clWINDOW,  clWINDOW);
    	gOutline(x1+1,y1+1,  x2-1,  y2-1, clWINDOW,  clWINDOW);
    	gROutline(++x1,++y1,  x2,  y2, clBORDER,  clBORDER);

    	gOutline(++x1,++y1,--x2,--y2, 0x00, 0x9B);//0x00, 0x77);
    	gOutline(++x1,++y1,--x2,--y2, 0x31,0xB1);//0x08, 0x88);
    } else {                          
    	if(flags&dbfTRANSHADOW)
        	gDrawShadow(x1,y1,x2,y2);
        else
        	gDrawShadowSolid(x1,y1,x2,y2,0x78);
		gRect(x1+2,y1+2,x2-2,y2-2, bnCols[0]);

    	gROutline(x1,y1,--x2,--y2, clBORDER,  clBORDER);

    	gOutline(++x1,++y1,--x2,--y2, bnCols[1], bnCols[2]);
    	gOutline(++x1,++y1,--x2,--y2, bnCols[3], bnCols[4]);
    }
}
/******************************************************************************/

void gDrawInsetFrame(WND *w,U8 wndCl)
{
	int x1=w->rect.left, y1=w->rect.top, x2=w->rect.right, y2=w->rect.bottom;

    U8 *inCols= (WndIsActive(w))?(U8*)inColsSel:(U8*)inColsReg;

    gROutline(  x1,  y1,  x2,  y2, inCols[1],  inCols[2]);
    gOutline(++x1,++y1,--x2,--y2, inCols[3],  inCols[4]);
    gRect(++x1,++y1,--x2,--y2, wndCl);
}
/******************************************************************************/

void gDrawListboxFrame(WND *w)
{
	int x1=w->rect.left, y1=w->rect.top, x2=w->rect.right, y2=w->rect.bottom;
    U8 *inCols= (WndIsActive(w))?(U8*)inColsSel:(U8*)inColsReg;
    gROutline(  x1,  y1,  x2,  y2, inCols[1],  inCols[2]);
    gOutline(++x1,++y1,--x2,--y2, inCols[3],  inCols[4]);
    if(w->ext.listbox.scrollbar)
    	x2-=wmxSCROLLBARWIDTH;
    gRect(++x1,++y1,--x2,--y2, clLISTBOX);
}
/******************************************************************************/
void gDrawEditFrame(WND *w)
{
 	_RECT oldPort;
    wGetPort(&oldPort);

    wSetPort(&w->parent->clRect);
    gRect(w->rect.left-2,w->rect.top-2,w->rect.right+2,w->rect.bottom+2, clWINDOW);

    gDrawInsetFrame(w,clEDIT);
    wSetPort(&w->clRect);
    DrawStringAbs(2,((S16)(GETCLIENTHEIGHT(w)-CHAR_HEIGHT))>>1,w->caption,0x00);
    if(w->style&wsSELECTABLE) {
    	wSetPort(&w->parent->clRect);
    	gDrawBmp(w->x+3+(w->ext.edit.col*CHAR_WIDTH),w->y-2,(BMP*)&bmpCounter);
    }
    wSetPort(&oldPort);
}
/******************************************************************************/
void gDrawBmp(int x, int y, BMP *b)
{
	int x1,x2=x+b->width,h=b->height;
    U8 *p=b->gData;
    while(h>0) {
    	x1=x;
    	while(x1<x2) {
        	if(*p)
            	PlotPix(x1,y,*p);
            x1++;
            p++;
        }
        h--;
        y++;
    }
}
/******************************************************************************/
void WndStopUpdate(WND *wf)
{
	WND *w;
    if(wf) {
    	w=wf->children;
    	while(w) {
    		WndStopUpdate(w);
    	    w=w->next;
    	}
    	wf->state |= wtNOPAINT|wtNOUPDATE;
    }
}
/******************************************************************************/
void WndStartCallback(WND *wf)
{
	WND *w;
    if(wf) {
    	w=wf->children;
    	while(w) {
    		WndStartCallback(w);
    	    w=w->next;
    	}
    	wf->state &= ~(wtNOPAINT|wtNOUPDATE);
    }
}
/******************************************************************************/
void WndStartUpdate(WND *wf)
{
    if(wf) {
		WndStartCallback(wf);
		wDrawWnd(wf,0);
    }
}
/******************************************************************************/
BOOL wEraseWnd(WND *w)
{
    _RECT oldPort;
	if(!w) return FALSE;
    wGetPort(&oldPort);

    //wSetPort((w->parent)?&w->parent->clRect:&scrRect);
	//gRect(w->rect.left, w->rect.top, w->rect.right, w->rect.bottom,0);

    wSetPort(&oldPort);
    SystemUpdate();

	return TRUE;
}
/******************************************************************************/
BOOL wDrawWnd(WND *w, U16 flags)
{
	WND *ch;
    _RECT oldPort;

    if((!GUI_ACTIVE)||(!w)||(w->state&wtNOPAINT)||(w->style&wsHIDDEN)) return FALSE;

    // if it's not the focused window, it can no longer be clicked
    if(w->prev&&(w->state&wtBTNDOWN))
    	w->state &= ~wtBTNDOWN;

    wGetPort(&oldPort);
    wSetPort(&w->parent->clRect);

	drawProcs[w->type](w,wdFULL);

    wSetPort(&w->clRect);
    WndMessage(w,wmPAINT,0,0,0);

    ch=w->children;
    while(ch) {
     	wDrawWnd(ch,wdFULL|wdNOUPDATE);
        ch=ch->next;
    }

    wSetPort(&oldPort);
    if(!(flags&wdNOUPDATE))
    	SystemUpdate();

	return TRUE;
}
/******************************************************************************/
BOOL wDrawWindow(WND *w, U16 flags)
{
	gDrawWindowFrame(w);
	return TRUE;
}
/******************************************************************************/
BOOL wDrawButton(WND *w, U16 flags)
{
	int tx,ty,bx,by,bw=0;
 	_RECT oldPort;
    BMP *b;
    U8 f=0;

    if(WndIsActive(w))
    	f|=dbfSELECTED;
    if(w->state&wtBTNDOWN)
    	f|=dbfDOWN;

	gDrawButtonFrame(w->rect.left,w->rect.top,w->rect.right,w->rect.bottom,f);


    wGetPort(&oldPort);
    wSetPort(&w->clRect);

    tx = (w->style&bsCAPTION)?(strlen(w->caption)*CHAR_WIDTH):0;    
	b = (BMP*)(w->ext.button.bitmap);
    bw = (w->style&bsBITMAP)?b->width+tx+4:tx;
    bx = ((signed int)(GETCLIENTWIDTH(w)-bw))>>1;

    if(w->style&bsBITMAP) {
    	by = ((signed int)(GETCLIENTHEIGHT(w)-b->height))>>1;
        if(w->state&wtBTNDOWN)
        	gDrawBmp(++bx,++by,b);
        else
        	gDrawBmp(bx,by,b);
    }

    if(w->style&bsCAPTION) {
    	tx = (bx+bw-tx);
    	ty = ((signed int)(GETCLIENTHEIGHT(w)-CHAR_HEIGHT))>>1;
    	if(w->state&wtBTNDOWN)
    		DrawStringAbs(++tx,++ty,w->caption,clBTNTEXTDOWN);
   		else
    		DrawStringAbs(tx,ty,w->caption,clBTNTEXT);
    }
    wSetPort(&oldPort);

	return TRUE;
}
/******************************************************************************/

BOOL wDrawText(WND *w, U16 flags)
{
    U8 *bnCols=(WndIsActive(w))?(U8*)bnColsSel:(U8*)bnColsReg;
	int x1=w->rect.left, y1=w->rect.top, x2=w->rect.right, y2=w->rect.bottom;
    _RECT oldPort;

    if(!w->caption) return FALSE;

    wGetPort(&oldPort);
    wSetPort(&w->clRect); 

    if(w->style&wsSELECTABLE)
    	gOutline(  x1,  y1,  x2,  y2, bnCols[1],  bnCols[2]);
	DrawStringAbs(0,0,w->caption,clWINDOWTEXT);
    wSetPort(&oldPort);
	return TRUE;
}
/******************************************************************************/

BOOL wDrawEdit(WND *w, U16 flags)
{
	gDrawEditFrame(w);
	return TRUE;
}
/******************************************************************************/

BOOL wDrawListBox(WND *w, U16 flags)
{
 	_RECT oldPort;
    int maxitems,y,cw,ch,topindex,bottomindex,actindex;
    LISTITEM *li;

    if(flags&wdFULL)
    	gDrawListboxFrame(w);

    if((topindex = LIFindIndex(w,w->ext.listbox.itemTop))==-1)
    	return TRUE;
    if((actindex = LIFindIndex(w,w->ext.listbox.itemActive))==-1)
    	return TRUE;
	maxitems = GETCLIENTHEIGHT(w)/CHAR_HEIGHT-1;
    bottomindex = topindex+maxitems;
    if(bottomindex<actindex) bottomindex=actindex;
    if(bottomindex<topindex) bottomindex=topindex;
    if(topindex<0) topindex=0;
    if(bottomindex>=w->ext.listbox.itemCount)
    	bottomindex=w->ext.listbox.itemCount-1;

    if((actindex-topindex)*CHAR_HEIGHT>(GETCLIENTHEIGHT(w)-CHAR_HEIGHT)) {
     	topindex++;
    }

    wGetPort(&oldPort);
    wSetPort(&w->clRect);

    cw = GETCLIENTWIDTH(w)-1;
    if(w->ext.listbox.scrollbar&&(w->ext.listbox.itemCount-1>maxitems))
    	cw-=wmxSCROLLBARWIDTH;
    ch = GETCLIENTHEIGHT(w);
    gRect(0,0,cw-1,ch-1,clLISTBOX);

    li = w->ext.listbox.itemTop = LIFind(w,topindex);
    y=2;
	while(li&&topindex<=bottomindex) {
    	if(li==w->ext.listbox.itemActive) {
        	gRect(0,y,cw,y+CHAR_HEIGHT-1,clHIGHLIGHT);
    		DrawStringAbs(2,y,li->text,clWHITE);
        } else
    		DrawStringAbs(2,y,li->text,clBLACK);
        li=li->next;
        y+=CHAR_HEIGHT;
        bottomindex--;
    }

    if(w->ext.listbox.scrollbar) {
    	w->ext.listbox.scrollbar->ext.scrollbar.max = w->ext.listbox.itemCount-maxitems-1;
        w->ext.listbox.scrollbar->ext.scrollbar.position = topindex;
    }

    wSetPort(&oldPort);

	return TRUE;
}
/******************************************************************************/

BOOL wDrawScrollBar(WND *w, U16 flags)
{
	int size,pos,cwidth=GETCLIENTWIDTH(w),cheight=GETCLIENTHEIGHT(w);
    float dSize,dPos,dPosSize;
    _RECT oldPort;
    U8 f=dbfTRANSHADOW;

    if(WndIsActive(w))
    	f|=dbfSELECTED;


	if(w->ext.scrollbar.max<w->ext.scrollbar.min)
    	w->ext.scrollbar.max = w->ext.scrollbar.min;
    if(w->ext.scrollbar.position<w->ext.scrollbar.min)
    	w->ext.scrollbar.position=w->ext.scrollbar.min;
    else if(w->ext.scrollbar.position>w->ext.scrollbar.max)
    	w->ext.scrollbar.position=w->ext.scrollbar.max;
    if(w->ext.scrollbar.min==w->ext.scrollbar.max)
    	return TRUE;

    if(w->style&sbBORDER)
    	gDrawInsetFrame(w,clSCROLLBAR);
    else
    	gRect(w->rect.left, w->rect.top, w->rect.right, w->rect.bottom,clSCROLLBAR);

 	size	= (w->ext.scrollbar.max+1)-w->ext.scrollbar.min;
	pos		= w->ext.scrollbar.position-w->ext.scrollbar.min;
    if(size<1) return FALSE;
    if(pos<0)
    	pos=0;

    dSize	= (float)((w->style&sbHORIZONTAL)?cwidth:cheight);
    dPosSize= ((dSize-size)<wmxMINSCROLLTAB)?wmxMINSCROLLTAB:dSize-size;
    dPos	= pos/(size/(dSize-dPosSize));

    wGetPort(&oldPort);
    wSetPort(&w->clRect);

	if(w->style&sbHORIZONTAL)
    	gDrawButtonFrame((int)dPos,0,(int)(dPos+dPosSize),cheight,f);
    else
    	gDrawButtonFrame(0,(int)dPos,cwidth,(int)(dPos+dPosSize),f);

    wSetPort(&oldPort);

	return TRUE;
}
/******************************************************************************/
const char *keynames[6][2][16] = {
	{{"ESC ","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10", ""},
	 {"ESC ","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10", ""}},

	{{"~ ","!","@","#","$","%","^","&","*","(",")","_","+","|", "  <<  ", ""},
	 {"` ","1","2","3","4","5","6","7","8","9","0","-","=","\\","  <<  ", ""}},

	{{"TAB ","Q","W","E","R","T","Y","U","I","O","P","{","}", ""},
	 {"TAB ","q","w","e","r","t","y","u","i","o","p","[","]", ""}},

	{{"caps ","A","S","D","F","G","H","J","K","L",":","\""," ENTER ", ""},
	 {"CAPS ","a","s","d","f","g","h","j","k","l",";","'", " ENTER ", ""}},

	{{"shift ","Z","X","C","V","B","N","M","<",">","?","  shift ", ""},
	 {"SHIFT ","z","x","c","v","b","n","m",",",".","/","  SHIFT ", ""}},

	{{"CTRL","ALT","      SPACE      ","ALT","CTRL", ""},
	 {"CTRL","ALT","      SPACE      ","ALT","CTRL", ""}},
};
/******************************************************************************/
// BAH! TEEEEEEEEEEEEEEEEDIOUS and a half!
const U16 keycodes[6][4][16] = {
	{{27,59<<8,60<<8,61<<8,62<<8,63<<8,64<<8,65<<8,66<<8,67<<8,68},
     {27,84<<8,85<<8,86<<8,87<<8,88<<8,89<<8,90<<8,91<<8,92<<8,93},
	 {27,94<<8,95<<8,96<<8,97<<8,98<<8,99<<8,100<<8,101<<8,102<<8,103},
     {27,104<<8,105<<8,106<<8,107<<8,108<<8,109<<8,110<<8,111<<8,112<<8,113}},

	{//{0x29<<8,2<<8,3<<8,4<<8,5<<8,6<<8,7<<8,8<<8,9<<8,10<<8,11<<8,12<<8,13<<8,0x2B<<8,8},
     {'\'','1','2','3','4','5','6','7','8','9','0','-','=','\\',8},
     {'~','!','@','#','$','%','^','&','&','(',')','_','=','|',8},
	 {0x29<<8,2<<8,3<<8,4<<8,5<<8,6<<8,7<<8,8<<8,9<<8,10<<8,11<<8,12<<8,13<<8,0x2B<<8,8},
     {0x29<<8,120<<8,121<<8,122<<8,123<<8,124<<8,125<<8,126<<8,127<<8,128<<8,129<<8,130<<8,131<<8,8}},

	//{{9,0x10<<8,0x11<<8,0x12<<8,0x13<<8,0x14<<8,0x15<<8,0x16<<8,0x17<<8,0x18<<8,0x19<<8,0x1A<<8,0x1B<<8},
	{{9,'q','w','e','r','t','y','u','i','o','p','[',']'},
     {9,'Q','W','E','R','T','Y','U','I','O','P','{','}'},
     {9,17,23,5,18,20,25,9,15,16,0<<8,0},
     {9,16<<8,17<<8,18<<8,19<<8,20<<8,21<<8,22<<8,23<<8,24<<8,25<<8,0<<8,}},

	//{{KEY_CAPSLOCK<<8,0x1E<<8,0x1F<<8,0x20<<8,0x21<<8,0x22<<8,0x23<<8,0x24<<8,0x25<<8,0x26<<8,0x27<<8,0x28<<8,13},
	{{KEY_CAPSLOCK,'a','s','d','f','g','h','j','k','l',';','\'',13},
     {KEY_CAPSLOCK,'A','S','D','F','G','H','J','K','L',':','"',13},
     {KEY_CAPSLOCK<<8,1,19,4,6,7,8,10,11,12,0<<8,0<<8,13},
     {KEY_CAPSLOCK<<8,30<<8,31<<8,32<<8,33<<8,34<<8,35<<8,36<<8,37<<8,38<<8,0<<8,0<<8,13}},

	//{{KEY_SHIFT,0x2C<<8,0x2D<<8,0x2E<<8,0x2F<<8,0x30<<8,0x31<<8,0x32<<8,0x33<<8,0x34<<8,0x35<<8,0x35<<8,KEY_SHIFT},
	{{KEY_SHIFT,'z','x','c','v','b','n','m',',','.','/',KEY_SHIFT},
     {KEY_SHIFT,'Z','X','C','V','B','N','M','<','>','?',KEY_SHIFT},
     {KEY_SHIFT,26,24,3,22,2,14,13,0<<8,0<<8,0<<8,KEY_SHIFT},
     {KEY_SHIFT,44<<8,45<<8,46<<8,47<<8,48<<8,49<<8,50<<8,0<<8,0<<8,0<<8,KEY_SHIFT}},

	{{KEY_CTRL,KEY_ALT,32,KEY_ALT,KEY_CTRL},
     {KEY_CTRL,KEY_ALT,32,KEY_ALT,KEY_CTRL},
     {KEY_CTRL,KEY_ALT,32,KEY_ALT,KEY_CTRL},
     {KEY_CTRL,KEY_ALT,32,KEY_ALT,KEY_CTRL}},
};
/*****************************************************************************/

int statekeys[] = {0,KEY_SHIFT,KEY_CTRL,KEY_ALT};
BOOL wDrawKeyboard(WND *w, U16 flags)
{
    U8 *bnCols=(WndIsActive(w))?(U8*)bnColsSel:(U8*)bnColsReg;
	int x1=w->rect.left, y1=w->rect.top, x2=w->rect.right, y2=w->rect.bottom;
    _RECT oldPort;

    if(!w->caption) return FALSE;

    wGetPort(&oldPort);
    wSetPort(&w->clRect);

    if(w->style&wsSELECTABLE)
    	gOutline(  x1,  y1,  x2,  y2, bnCols[1],  bnCols[2]);
    {
    int state = w->ext.keyboard.state;
    int selrow = w->ext.keyboard.row;
    int selcol = w->ext.keyboard.col;
    int y = 1;
    int row;
	for(row=0;row<6;row++) {
        const char **s = keynames[row][state==KEYSTATE_SHIFT?0:1];
        int x = 1;
        int col=0;
        int w;
        while(**s) {
            w = (strlen((char*)*s))*CHAR_WIDTH+(CHAR_WIDTH);
			if(row==selrow&&col==selcol) {
            	DrawStringAbsBG(x,y,(char*)*s,cl3DLIGHTDK,clKEYSEL);
            } else {
            	if((state) && (keycodes[row][state][col]==statekeys[state]) )
            		DrawStringAbsBG(x,y,(char*)*s,clWINDOWTEXT,cl3DSHADOWDK);
                else
            		DrawStringAbsBG(x,y,(char*)*s,clWINDOWTEXT,cl3DLIGHTDK);
           	}
            x += w;
            s++;
            col++;
        }
        y += CHAR_HEIGHT+1;
    }
    }
    wSetPort(&oldPort);
	return TRUE;
}
/******************************************************************************/
void KeyboardSelectKey(WND *w, int xd, int yd)
{
	int rowlen;
    const char **s;

    int row = w->ext.keyboard.row;
    int col = w->ext.keyboard.col;
    int state = w->ext.keyboard.state;

	if((row+=yd)>=6) row = 0;
    else if(row<0) row = 5;

	rowlen = 0;
	s = keynames[row][state==KEYSTATE_SHIFT?0:1];
    while(**s) { rowlen++; s++; }

	if((col+=xd)>=rowlen) col = 0;
    else if(col<0) col = rowlen-1;

    w->ext.keyboard.col = col;
    w->ext.keyboard.row = row;

	w->ext.keyboard.selkey = keycodes[row][state][col];

    wDrawWnd(w, 0);
}
/******************************************************************************/

/******************************************************************************/
BOOL wmActivate(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
BOOL wmDeactivate(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
BOOL wmShow(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
BOOL wmHide(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
BOOL wmPaint(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
BOOL wmButtonPress(WND *w, U16 wParam, void *pParam, U16 flags)
{
	switch(w->type) {
    	case wnEDIT:
			w->state|=wtBTNDOWN;
         	switch(wParam) {
            	case KEY_UP:
                    EditScrollChar(w,+1);
                    break;
            	case KEY_DOWN:
                    EditScrollChar(w,-1);
                    break;
            	case KEY_LEFT:
                    EditScrollCol(w,-1,FALSE);
                    break;
            	case KEY_RIGHT:
                    EditScrollCol(w,+1,FALSE);
                    break;
            	case KEY_ESC:
                    EditScrollCol(w,-1,TRUE);
                    break;
            }
            break;
    	case wnLISTBOX:
			w->state|=wtBTNDOWN;
         	switch(wParam) {
            	case KEY_UP:
                    ListBoxPrevItem(w);
                    break;
            	case KEY_DOWN:
                    ListBoxNextItem(w);
                    break;
            	case KEY_LEFT:
                    ListBoxPrevItemCh(w);
                    break;
            	case KEY_RIGHT:
                    ListBoxNextItemCh(w);
                    break;
            }
#ifdef _WINDOWS
    		Delay(19);
#else
    		Delay(16);
#endif
            break;
    	case wnKEYBOARD:
			w->state|=wtBTNDOWN;
         	switch(wParam) {
            	case KEY_UP:
                   	KeyboardSelectKey(w,0,-1);
                    break;
            	case KEY_DOWN:
                   	KeyboardSelectKey(w,0,+1);
                    break;
            	case KEY_LEFT:
                   	KeyboardSelectKey(w,-1,0);
                    break;
            	case KEY_RIGHT:
                   	KeyboardSelectKey(w,+1,0);
                    break;
                case KEY_ENTER:
    				if(w->ext.keyboard.selkey==KEY_SHIFT) {
    					w->ext.keyboard.state =
                        	(w->ext.keyboard.state == KEYSTATE_SHIFT)?
                            	KEYSTATE_NONE:KEYSTATE_SHIFT;
    					wDrawWnd(w, 0);
    				} else if(w->ext.keyboard.selkey==KEY_CTRL) {
    					w->ext.keyboard.state =
                        	(w->ext.keyboard.state == KEYSTATE_CTRL)?
                            	KEYSTATE_NONE:KEYSTATE_CTRL;
    					wDrawWnd(w, 0);
    				} else if(w->ext.keyboard.selkey==KEY_ALT) {
    					w->ext.keyboard.state =
                        	(w->ext.keyboard.state == KEYSTATE_ALT)?
                            	KEYSTATE_NONE:KEYSTATE_ALT;
    					wDrawWnd(w, 0);
    				} else
     					WndMessage(w, wmKEYBOARD_INPUT, w->ext.keyboard.selkey, 0, 0);
                    break;
            }
    		Delay(20);
            break;
    	case wnSCROLLBAR:
			w->state|=wtBTNDOWN;
         	switch(wParam) {
            	case KEY_UP:
            	case KEY_LEFT:
                    if(w->ext.scrollbar.position>w->ext.scrollbar.min) {
                     	w->ext.scrollbar.position--;
                        if(!(flags&mtPARENT)) wDrawWnd(w,0);
                    }
                    break;
            	case KEY_RIGHT:
            	case KEY_DOWN:
                    if(w->ext.scrollbar.position<w->ext.scrollbar.max) {
                     	w->ext.scrollbar.position++;
                        if(!(flags&mtPARENT)) wDrawWnd(w,0);
                    }
                    break;
            }
            break;
    	case wnBUTTON:
         	switch(wParam) {
            	case KEY_ENTER:
            	case KEY_ESC:
                	break;
                default:
                	return TRUE;
            }
        default:
			w->state|=wtBTNDOWN;
			if(!(flags&mtPARENT)) wDrawWnd(w,wdFULL);
    }
	return TRUE;
}
/******************************************************************************/
BOOL wmButtonRelease(WND *w, U16 wParam, void *pParam, U16 flags)
{           
	if(w->state&wtBTNDOWN) {
		w->state&=~wtBTNDOWN;
        WndMessage(w,wmBUTTON_CLICK,wParam,pParam,flags);
    }
	return TRUE;
}
/******************************************************************************/
BOOL wmButtonClick(WND *w, U16 wParam, void *pParam, U16 flags)
{
	switch(w->type) {
    	case wnLISTBOX:
        	if(wParam==KEY_ENTER||wParam==KEY_ESC) {
            	WndMessage(w,wmLISTBOX_CLICK,wParam,(void*)LIGetSelText(w),flags);
            	break;
            }
        case wnBUTTON:
        	if(wParam!=KEY_ENTER&&wParam!=KEY_ESC)
            	return 0;
        default:
       		if(!(flags&mtPARENT))
            	wDrawWnd(w,wdFULL);
            break;
    }
	return 1;
}
/******************************************************************************/
BOOL wmButtonHold(WND *w, U16 wParam, void *pParam, U16 flags)
{
	if(wParam==KEY_UP||wParam==KEY_DOWN||wParam==KEY_LEFT||wParam==KEY_RIGHT)//w->type==wnLISTBOX||w->type==wnSCROLLBAR)
    	return(wmButtonPress(w, wParam, pParam,flags));
	return TRUE;
}
/******************************************************************************/
BOOL wmListboxClick(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
BOOL wmListboxChange(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
BOOL wmEditChange(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}  
/******************************************************************************/
BOOL wmKeyboardInput(WND *w, U16 wParam, void *pParam, U16 flags)
{
	return TRUE;
}
/******************************************************************************/
S16 wnBaseProc(struct _WND *w, U16 msg, U16 wParam, void *pParam)
{
	return 1;
}
/******************************************************************************/
void WndDisposeCallback(WND *wf)
{
	WND *w,*nx;
    if(wf) {
    	w=wf->children;
    	while(w) {
    	    nx=w->next;
    		WndDisposeCallback(w);
            w=nx;
    	}
        if(wf->style&wsEXTFIXED) {
        } else {
       		if(wf->type==wnLISTBOX)
            	ListBoxClear(wf);
        	else
            	memset(&wf->ext,0,sizeof(wf->ext));
        }
    	wf->prev=NULL;
    	wf->next=NULL;
    	wf->children=NULL;
    }
}
/******************************************************************************/
void WndDispose(WND *w)
{
	if(!w) return;

	WndStopUpdate(w);

	if(w==wnActive)
		WndBringToFront(WndSelectNext(w));

    if(w&&w->parent)
    	w->parent->children=wnActive;

    if(wnActive && ((wnActive->style&wsDISABLED)||(wnActive->style&wsHIDDEN)))
    	wnActive = NULL;

	if(w->prev)
		w->prev->next = w->next;
	if(w->next)
		w->next->prev = w->prev;

	WndDisposeCallback(w);

	wEraseWnd(w);
	RedrawAllWindows();
}
/******************************************************************************/
U8 wMessageBox(char *title, char *text, U8 buttons)
{
	char *s;
    int x,y,r,c,cwidth,cheight;
    _RECT oldPort;

    if(buttons>=mbMAX)
    	return 0;

	maxWidth	= 30;

    s = WordWrap(text);

    wnMsgBox.width		= (msgWidth+2)*CHAR_WIDTH+(wmxBORDERFIXED<<1);
    wnMsgBox.height		= ((msgHeight+2)*CHAR_HEIGHT)+wmxTITLEHEIGHT+(wmxBORDERFIXED<<1)+20;
    wnMsgBox.x			= ((SCREEN_WIDTH-wnMsgBox.width)>>1);
    wnMsgBox.y			= ((SCREEN_HEIGHT-wnMsgBox.height)>>1);
    wnMsgBox.caption	= title;

    AddWindow(&wnMsgBox);
    cwidth				= GETCLIENTWIDTH(&wnMsgBox);
    cheight				= GETCLIENTHEIGHT(&wnMsgBox);
    x					= (cwidth-btnMsgWide[buttons])>>1;
    y					= cheight-20;

    if(buttons==mbOK||buttons==mbOKCANCEL) {
    	bnMsgOK.x			= x;
    	bnMsgOK.y			= y;
        AddWindow(&bnMsgOK);
        x+=34;
    }
    if(buttons==mbYESNO||buttons==mbYESNOCANCEL) {
    	bnMsgYes.x			= x;
    	bnMsgYes.y			= y;
        AddWindow(&bnMsgYes);
        x+=34;
    	bnMsgNo.x			= x;
    	bnMsgNo.y			= y;
        AddWindow(&bnMsgNo);
        x+=34;
    }
    if(buttons==mbOKCANCEL||buttons==mbYESNOCANCEL) {
    	bnMsgCancel.x		= x;
    	bnMsgCancel.y	   	= y;
        AddWindow(&bnMsgCancel);
    }

    wGetPort(&oldPort);
    wSetPort(&wnMsgBox.clRect);

    r = 8;
    y = msgHeight;
    while(y) {
    	c = 8;
        while(*s&&*s!='\n') {
        	AbChar(*s++, c, r, clBLACK,clWINDOW);
            c+=CHAR_WIDTH;
        }
        s++;
        r+=CHAR_HEIGHT;
    	y--;
    }
    wSetPort(&oldPort);

    maxWidth		= -1;

    WinGUIDoit();

    return 0;
}
/******************************************************************************/
S16 wnMsgBoxProc(struct _WND *w, U16 msg, U16 wParam, void *pParam)
{
	switch(msg) {
    	case wmBUTTON_CLICK:
        	if(w==&bnMsgOK) {
            	msgResult = idOK;
             	WndDispose(&wnMsgBox);
            } else if(w==&bnMsgYes) {
            	msgResult = idYES;
             	WndDispose(&wnMsgBox);
            } else if(w==&bnMsgNo) {
            	msgResult = idNO;
             	WndDispose(&wnMsgBox);
            } else if(w==&bnMsgCancel) {
            	msgResult = idCANCEL;
             	WndDispose(&wnMsgBox);
            }
            break;
    }
	return TRUE;
}
/******************************************************************************/
S16 wnDesktopProc(struct _WND *w, U16 msg, U16 wParam, void *pParam)
{
	return 1;
}
/******************************************************************************/
S16 wnProc2(struct _WND *w, U16 msg, U16 wParam, void *pParam)
{
	if(w==&bnBtn1&&msg==wmBUTTON_CLICK) {
    	WndDispose(&wnTest);
	}
	return 1;
}
/******************************************************************************/
S16 wnProc(struct _WND *w, U16 msg, U16 wParam, void *pParam)
{
	if(w==&bnBtn1&&msg==wmBUTTON_CLICK) {
        	WndDispose(&wnTest);
	}
	return 1;
}
/******************************************************************************/
void RedrawAllWindows()
{
    WND *w;

    if(!GUI_ACTIVE) return;

    w = GetWndLast(wnFirst);
    while(w) {
    	wDrawWnd(w,wdFULL);
     	w=w->prev;
    }
}
/******************************************************************************/
void wHideWindow(WND *w)
{
	w->style |= wsHIDDEN;

	if(w==wnActive)
		WndBringToFront(WndSelectNext(w));

	wEraseWnd(w);
}
/******************************************************************************/
void wShowWindow(WND *w)
{
	w->style &= ~wsHIDDEN;
	RedrawAllWindows();
}
/******************************************************************************/
void wDisableWindow(WND *w)
{
	w->style |= wsDISABLED;

	if(w==wnActive)
		WndBringToFront(WndSelectNext(w));

	wEraseWnd(w);
}
/******************************************************************************/
void wEnableWindow(WND *w)
{
	w->style &= ~wsDISABLED;
	RedrawAllWindows();
}
/******************************************************************************/


