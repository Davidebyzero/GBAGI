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
#ifndef _TEXT_H
#define _TEXT_H
/*****************************************************************************/
#define CHAR_WIDTH		6
#define CHAR_HEIGHT		8

#define MAX_MSG_SIZE	1000 // (40*25)
#define MAX_MSG_WIDTH	30
#define MAX_MSG_HEIGHT	20
extern char msgBuf[MAX_MSG_SIZE+1];
extern char msgBuf2[MAX_MSG_SIZE+1];

extern const U8 fontData[256*8];
extern int msgX,msgY,msgHeight,msgWidth,maxWidth,ydiff,txYOffs;
extern _RECT wndDraw;
/*****************************************************************************/
#define SET_ROWCOL(r,c) { textRow = (r); textCol = (c);}   
#define PUSH_TEXT_STYLE()\
	U8 oldAttr=textAttr,oldColour=textColour,oldRow=textRow,oldCol=textCol;
#define POP_TEXT_STYLE()\
	textAttr=oldAttr;textColour=oldColour;textRow=oldRow;textCol=oldCol;

void AbChar(U8 c, int abX, int abY, U8 t1, U8 t2);
void DrawChar(char c,BOOL FIX);
void DrawStringAbs(int x, int y, char *s, U8 c);  
void DrawStringAbsBG(int x, int y, char *s, U8 c, U8 bg);
void DrawString(char *s,BOOL FIX);
void DrawAGIString(char *s);
char *FormatAGIString(char *s,char *out);
BOOL MessageBox(char *szMsg);
BOOL MessageBoxXY(char *szMsg, S8 row, S8 col, S8 width);

char *WordWrap(char *s);
/*****************************************************************************/
#endif
/*****************************************************************************/
