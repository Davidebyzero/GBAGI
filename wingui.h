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
#ifndef _WINGUI_H
#define _WINGUI_H
/*****************************************************************************/
#include "gbagi.h"
#include "input.h"
/*****************************************************************************/
typedef U16	HDL;
#define HDL_MAX	65534
#define HDL_NULL	65535

/*****************************************************************************/
typedef struct {
	U8 width,height;
	U8 *gData;
} BMP;

#define lfINITIALIZED	0x8000

typedef struct _LISTITEM {
	struct _LISTITEM *prev,*next;
    U16 index;
	char *text;
} LISTITEM;

#define MAX_LISTITEMS	512
/*****************************************************************************/
struct _WND;
typedef S16 (*WNPROC)(struct _WND *w, U16 msg, U16 wParam, void *pParam);
typedef struct _WND {
	struct _WND *prev,*next,*parent,*children;
	//HDL id,chId;
	S16 x,y,width,height;
	_RECT rect,clRect;
	char *caption;
	U8 type;
	U16 state; // state bits
	U16 style; // property bits
	WNPROC proc;
	union {
		struct {
			int i;
		} window;
		struct {
			BMP *bitmap;
		} button;
		struct {
			S16 i;
		} text;
		struct {
			S16 row,col,maxLen;
		} edit;
		struct {
			U16 itemCount;
			LISTITEM *itemFirst,*itemLast,*itemActive,*itemTop;
            struct _WND *scrollbar;
		} listbox;
		struct {
			S16 min,max,position;
		} scrollbar;
		struct {
        	U8 row, col;
			U16 selkey;
            U8 state;
		} keyboard;
	} ext;
} WND;
/*****************************************************************************/
/*****************************************************************************/
#define wnWINDOW			0
#define wnBUTTON			1
#define wnTEXT				2
#define wnEDIT				3
#define wnLISTBOX			4
#define wnSCROLLBAR			5
#define wnKEYBOARD			6
#define wnTOTAL				7
/*****************************************************************************/
#define wdFULL				0x0001
#define wdNOUPDATE			0x0002
/*****************************************************************************/
#define wtNOPAINT			0x0001
#define wtBTNDOWN			0x0002
#define wtNOUPDATE			0x0004
/*****************************************************************************/
#define wsDISABLED			0x0001
#define wsHIDDEN			0x0002
#define wsSELECTABLE		0x0004

#define wsEXTFIXED			0x0008


#define wsBORDER			0x0010
#define wsTITLE				0x0020
#define wsRESIZABLE			0x0040 // includes a border
#define wsMODAL				0x0080

#define bs3STATE			0x0010
#define bsRADIO				0x0020
#define bsCAPTION			0x0040
#define bsICON				0x0080
#define bsBITMAP			0x0100

#define esMULTILINE			0x0010  
#define esDIGITONLY			0x0020

#define txAUTOSIZE			0x0010	// the initial width and height are ignored and set to the exact size of the text plus any borders and such

#define sbBORDER			0x0010
#define sbHORIZONTAL		0x0020	// horizontal scrollbar
#define sbVERTICAL			0x0000	// vertical scrollbar


// window metrics
#define wmxBORDERFIXED		2
#define wmxBORDERRESIZE		4
#define wmxBUTTONBORDER		3
#define wmxSCROLLBARWIDTH	10
#define wmxSCROLLBORDER		1
#define wmxINSETBORDER		2
#define wmxTEXTBORDER		1
#define wmxTITLEHEIGHT		12

#define wmxMINSCROLLTAB		10

/*****************************************************************************/

typedef BOOL (*DRAWPROC)(WND *w, U16 flags);

BOOL wDrawWnd(WND *w, U16 flags);

BOOL wDrawWindow(WND *w, U16 flags);
BOOL wDrawButton(WND *w, U16 flags);
BOOL wDrawText(WND *w, U16 flags);
BOOL wDrawEdit(WND *w, U16 flags);
BOOL wDrawListBox(WND *w, U16 flags);
BOOL wDrawScrollBar(WND *w, U16 flags);
BOOL wDrawKeyboard(WND *w, U16 flags);

extern const DRAWPROC drawProcs[wnTOTAL];

/*****************************************************************************/

#define wmACTIVATE			1
#define wmDEACTIVATE		2
#define wmSHOW				3
#define wmHIDE				4
#define wmPAINT				5
#define wmBUTTON_PRESS		6
#define wmBUTTON_RELEASE	7
#define wmBUTTON_CLICK		8
#define wmBUTTON_HOLD		9
#define wmLISTBOX_CLICK		10
#define wmLISTBOX_CHANGE	11
#define wmEDIT_CHANGE		12
#define wmKEYBOARD_INPUT	13
#define wmMAX				14

#define mtPARENT			0x0001

BOOL wmActivate(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmDeactivate(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmShow(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmHide(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmPaint(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmButtonPress(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmButtonRelease(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmButtonClick(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmButtonHold(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmListboxClick(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmListboxChange(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmEditChange(WND *w, U16 wParam, void *pParam, U16 flags);
BOOL wmKeyboardInput(WND *w, U16 wParam, void *pParam, U16 flags);

typedef BOOL (*MSGPROC)(WND *w, U16 wParam, void *pParam, U16 flags);

typedef struct {
 	MSGPROC proc;
    U16 flags;
} MSGNODE;
extern const MSGNODE msgProcs[wmMAX];
/*****************************************************************************/

void InitWinGUISystem(BOOL RESTART);
int WinGUIDoit(void);
WND *WndBringToFront(WND *wNewActive);
WND *GetWndLast(WND *parent);
void wSetPort(_RECT *r);
void wGetPort(_RECT *r);
BOOL WndCreate(WND *w, WND *parent, U8 type, S16 x, S16 y, S16 width, S16 height, U16 style, char *caption, WNPROC proc);
BOOL AddWindow(WND *w);
WND *GetFirstWnd(void);
WND *GetActiveWnd(WND *wfirst);
WND *GetWndLast(WND *parent);
void CalcWndRect(WND *w);
HDL GetNextID(WND *parent);
BOOL IsOnlyChild(WND *w);  
WND *WndSelect(WND *wSelect,int direction);
WND *WndSelectPrev(WND *wStart);
WND *WndSelectNext(WND *wFirst);
WND *GetWndLastSel(WND *wStart);
WND *GetWndFirstSel(WND *wEnd);
WND *GetWndNextSel(WND *wStart);
WND *GetWndPrevSel(WND *wStart);
S16 WndMessage(WND *w, U32 msg, U16 wParam, void *pParam, U16 flags);

BOOL WndIsActive(WND *w);      

U8 wMessageBox(char *title, char *text, U8 buttons);

#define mbOK				0
#define mbOKCANCEL			1
#define mbYESNO				2
#define mbYESNOCANCEL		3
#define mbMAX				4

#define idOK				0x01
#define idYES				0x01
#define idNO				0x02
#define idCANCEL			0x04

#define drFORWARD 		(+1)
#define drBACKWARD		(-1)
/*****************************************************************************/
#define clBLACK			0x00
#define clWHITE			0xFF
#define clSILVER		0x77
#define clLTSILVER		0xF7
#define clGREY			0x88
#define clDKGREY		0x08
#define clBLUE			0x99
#define clYELLOW		0xEE

#define clWINDOW		0xF7
#define clWINDOWTEXT	clBLACK
#define clBORDER		clBLACK
#define cl3DLIGHT		0xF7
#define cl3DSHADOW      0x88
#define cl3DLIGHTDK     0xFF
#define cl3DSHADOWDK   	0x77
#define clTITLEACTIVE	0x95
#define clTITLEINACTIVE	clGREY
#define clTITLETEXT		clWHITE
#define clBTNTEXT		clBLACK
#define clBTNTEXTDOWN	clWHITE
#define clHIGHLIGHT		0x95
#define clKEYSEL		0x99
#define clKEYSHIFT		0x99
#define clSCROLLBAR		0x70//0x88
#define clEDIT			clWHITE
#define clLISTBOX		clWHITE
/*****************************************************************************/
void gHLine(int x1, int x2, int y, U8 colour);
void gVLine(int y1, int y2, int x, U8 colour);
void gRect(int x1, int y1, int x2, int y2, U8 colour);
void gOutline(int x1, int y1, int x2, int y2, U8 hlgt, U8 shad);
void gDrawWindowFrame(WND *w);
void gDrawButtonFrame(int x1,int y1,int x2,int y2,U16 flags);
void gROutline(int x1, int y1, int x2, int y2, U8 hlgt, U8 shad);
void gDrawShadow(int x1, int y1, int x2, int y2);

void WndStopUpdate(WND *w);
void WndStartUpdate(WND *w);

#define dbfDOWN			0x01
#define dbfSELECTED		0x02
#define dbfTRANSHADOW	0x04

LISTITEM *LIMalloc(void);
void LIFree(LISTITEM *li);
S16 ListBoxAdd(WND *w, char *text);
BOOL ListBoxDel(WND *w, int index);
LISTITEM *LIFind(WND *w, int index);
S16 LIFindIndex(WND *w, LISTITEM *li);
void ListBoxPrevItem(WND *w);
void ListBoxNextItem(WND *w);
void ListBoxSelect(WND *w,int item);
void ListBoxSetScrollbar(WND *lb,WND *sb);
void ListBoxSelItem(WND *w,int diff);
char *LIGetSelText(WND *w);
void WndDispose(WND *w);
void ListBoxClear(WND *w);
void EditScrollChar(WND *w,int dir);
void EditScrollCol(WND *w,int dir, BOOL del);
void KeyboadSelectKey(WND *w, int xd, int yd);

void RedrawAllWindows(void);
void wHideWindow(WND *w);
void wShowWindow(WND *w);
void wDisableWindow(WND *w);
void wEnableWindow(WND *w);

void gDrawBmp(int x, int y, BMP *b);

#define GETCLIENTHEIGHT(w)\
	(((w)->clRect.bottom+1)-(w)->clRect.top)
#define GETCLIENTWIDTH(w)\
	(((w)->clRect.right+1)-(w)->clRect.left)

#define GETLBMAXVISIBLE(w)\
	(GETCLIENTHEIGHT(w)/CHAR_HEIGHT)

extern WND wnBase;
void  RunGUI1(void);
void FreeWinGUISystem(void);

extern BOOL GUI_ACTIVE;

enum {
	KEYSTATE_NONE	= 0x00,
	KEYSTATE_SHIFT	= 0x01,
	KEYSTATE_CTRL	= 0x02,
	KEYSTATE_ALT	= 0x03,
};
/*****************************************************************************/
#endif
/*****************************************************************************/
