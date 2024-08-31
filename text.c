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
#include "text.h"
#include "screen.h"
#include "errmsg.h"
#include "input.h"
#include "system.h"
#include "logic.h"
#include "commands.h"
#include "gamedata.h"
#include "wingui.h"
#include "parse.h"
/******************************************************************************/
const U8 fontData[256*8] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00, 
	0x6C, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00, 
	0x10, 0x7C, 0xD0, 0x7C, 0x16, 0xFC, 0x10, 0x00, 
	0x00, 0x66, 0xAC, 0xD8, 0x36, 0x6A, 0xCC, 0x00, 
	0x38, 0x4C, 0x38, 0x78, 0xCE, 0xCC, 0x7A, 0x00, 
	0x30, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00, 
	0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00, 
	0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, 
	0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x20, 
	0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 
	0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00, 
	0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xE6, 0x7C, 0x00, 
	0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x7E, 0x00, 
	0x7C, 0xC6, 0x06, 0x1C, 0x70, 0xC6, 0xFE, 0x00, 
	0x7C, 0xC6, 0x06, 0x3C, 0x06, 0xC6, 0x7C, 0x00, 
	0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00, 
	0xFE, 0xC0, 0xFC, 0x06, 0x06, 0xC6, 0x7C, 0x00, 
	0x7C, 0xC6, 0xC0, 0xFC, 0xC6, 0xC6, 0x7C, 0x00, 
	0xFE, 0xC6, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,
	0x7C, 0xC6, 0xC6, 0x7C, 0xC6, 0xC6, 0x7C, 0x00, 
	0x7C, 0xC6, 0xC6, 0x7E, 0x06, 0xC6, 0x7C, 0x00, 
	0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 
	0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x10, 0x20, 
	0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00, 
	0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00, 
	0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,
	0x78, 0xCC, 0x0C, 0x18, 0x30, 0x00, 0x30, 0x00, 
	0x7C, 0x82, 0x9E, 0xA6, 0x9E, 0x80, 0x7C, 0x00, 
	0x7C, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00, 
	0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00, 
	0x7C, 0xC6, 0xC0, 0xC0, 0xC0, 0xC6, 0x7C, 0x00, 
	0xFC, 0x66, 0x66, 0x66, 0x66, 0x66, 0xFC, 0x00, 
	0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00, 
	0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00, 
	0x7C, 0xC6, 0xC6, 0xC0, 0xCE, 0xC6, 0x7E, 0x00, 
	0xC6, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00, 
	0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 
	0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00, 
	0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00, 
	0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00, 
	0x82, 0xC6, 0xEE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00, 
	0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00,
	0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 
	0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00, 
	0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0xDE, 0x7C, 0x06,
	0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xE6, 0x00, 
	0x7C, 0xC6, 0xC0, 0x7C, 0x06, 0xC6, 0x7C, 0x00, 
	0x7E, 0x5A, 0x5A, 0x18, 0x18, 0x18, 0x3C, 0x00, 
	0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 
	0xC6, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x10, 0x00, 
	0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6, 0x82, 0x00, 
	0xC6, 0x6C, 0x38, 0x38, 0x38, 0x6C, 0xC6, 0x00, 
	0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x3C, 0x00,
	0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00, 
	0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00, 
	0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00, 
	0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00, 
	0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 
	0x30, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00, 
	0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00, 
	0x00, 0x00, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x00, 
	0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,
	0x00, 0x00, 0x7C, 0xC6, 0xFE, 0xC0, 0x7C, 0x00,
	0x1C, 0x36, 0x30, 0x78, 0x30, 0x30, 0x78, 0x00, 
	0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x78, 
	0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00, 
	0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00,
	0x00, 0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0x78, 
	0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00, 
	0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, 
	0x00, 0x00, 0xCC, 0xFE, 0xD6, 0xD6, 0xD6, 0x00, 
	0x00, 0x00, 0xDC, 0x66, 0x66, 0x66, 0x66, 0x00, 
	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00, 
	0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0, 
	0x00, 0x00, 0x7C, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E, 
	0x00, 0x00, 0xDE, 0x76, 0x60, 0x60, 0xF0, 0x00, 
	0x00, 0x00, 0x7C, 0xC0, 0x7C, 0x06, 0x7C, 0x00, 
	0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00, 
	0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00, 
	0x00, 0x00, 0xC6, 0xC6, 0x6C, 0x38, 0x10, 0x00, 
	0x00, 0x00, 0xC6, 0xD6, 0xD6, 0xFE, 0x6C, 0x00,
	0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00, 
	0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
	0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00, 
	0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E, 0x00, 
	0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,
	0xE0, 0x30, 0x30, 0x18, 0x30, 0x30, 0xE0, 0x00, 
	0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00,
0x1F,0x39,0x79,0x79,0x78,0x38,0x1F,0x00,
0xF0,0xF8,0xFC,0xFC,0x3C,0x38,0xF0,0x00,
0x1F,0x38,0x79,0x78,0x79,0x39,0x1F,0x00,
0xF0,0x78,0x3C,0x7C,0x3C,0x38,0xF0,0x00,
0x1F,0x3C,0x79,0x78,0x79,0x39,0x1F,0x00,
0xF0,0x78,0x3C,0x3C,0x3C,0x38,0xF0,0x00,
0x1F,0x38,0x79,0x78,0x79,0x38,0x1F,0x00,
0xF0,0x78,0x3C,0x7C,0x3C,0x78,0xF0,0x00,
0x00,0xCC,0x66,0x33,0x33,0x66,0xCC,0x00,
0x00,0x33,0x66,0xCC,0xCC,0x66,0x33,0x00,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
0x7E, 0x83, 0x93, 0x9F, 0x91, 0x99, 0x81, 0x7E
};           
/******************************************************************************/
char msgBuf[MAX_MSG_SIZE+1];
char msgBuf2[MAX_MSG_SIZE+1];
char nStr[8];
int msgX,msgY,msgHeight,msgWidth,maxWidth,ydiff,txYOffs;
_RECT wndDraw;
const int shadpix[9]={0x00,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55};
/******************************************************************************/
void AbTransChar(U8 c, int abX, int abY, U8 t1)
{
	U8 *p = (U8*)(&fontData[c<<3]);
    int x1,h=8;
    U8 c1 = t1;
    t1&=0x0F;

	while(h) {
        x1 = abX;

        if(*p&0x80) PlotPix(x1, abY, c1);
        x1++;
        if(*p&0x60) {
        	if((*p&0x60)==0x60)
        		PlotPix(x1, abY, c1);
            else  
        		PlotPixLO(x1, abY, t1);
        }
        x1++;
        if(*p&0x10) PlotPix(x1, abY, c1);
        x1++;
        if(*p&0x0C) {
        	if((*p&0x0C)==0x0C) 
        		PlotPix(x1, abY, c1);
            else          
        		PlotPixLO(x1, abY, t1);
        }
        x1++;
        if(*p&0x02) PlotPix(x1, abY, c1);
        x1++;
        if(*p&0x01) PlotPix(x1, abY, c1);
        x1++;

        p++;
        abY++;
        h--;
    }
}
/******************************************************************************/
void AbChar(U8 c, int abX, int abY, U8 t1, U8 t2)
{
	U8 *p = (U8*)(&fontData[c<<3]);
    int x1,h=8;
    U8 c1,b;

	while(h) {
        x1 = abX;
        b = *p++;

        if(textAttr&2)
        	b=~b;
        if(textAttr&1)
        	b&=shadpix[h];

        // "aa bc dd ef gg hh"  works best IMO, since the "hh" is usually
        //	blank, the center is really the "dd"

        c1 = (b & 0x80)? t1:t2;
        PlotPix(x1++, abY, (c1<<4)|c1);
        PlotPix(x1++,abY, ( ((b & 0x40)? t1:t2) <<4) | ((b & 0x20)? t1:t2));
        c1 = (b & 0x10)? t1:t2;
        PlotPix(x1++, abY, (c1<<4)|c1);
        PlotPix(x1++,abY, ( ((b & 0x08)? t1:t2) <<4) | ((b & 0x04)? t1:t2));
        c1 = (b & 0x02)? t1:t2;
        PlotPix(x1++, abY, (c1<<4)|c1);
        c1 = (b & 0x01)? t1:t2;
        PlotPix(x1, abY, (c1<<4)|c1);

        abY++;
        h--;
    }
}
/******************************************************************************/
int CalcTextY(int row,BOOL FIX)
{
	return (FIX&&!TEXT_MODE)?((row-(minRow?1:0))*CHAR_HEIGHT)-Y_ADJUST_CL:(row*CHAR_HEIGHT);
}
/******************************************************************************/
void DrawChar(char c,BOOL FIX)
{
	AbChar(c,
    	(textCol*CHAR_WIDTH),
        CalcTextY(textRow,FIX),
        (textColour&0xF),(textColour>>4)
    );
    textCol++;
    if(textCol==40) {
     	textCol = 0;
        textRow++;
    }
}
/******************************************************************************/
void DrawCharTrans(char c,BOOL FIX,U8 cl)
{
	AbTransChar(c,
    	(textCol*CHAR_WIDTH),
        CalcTextY(textRow,FIX),
        cl
    );
    textCol++;
    if(textCol==40) {
     	textCol = 0;
        textRow++;
    }
}
/******************************************************************************/
void DrawStringAbs(int x, int y, char *s, U8 c)
{
	int l=strlen(s);
    int x1=x;
	while(l) {                
        if(*s=='\n') {  
            x1 = x;
        	y+=CHAR_HEIGHT;
            if(!--l||*++s=='\0') break;
        }
    	AbTransChar(*s++,x1,y,c);
        x1+=CHAR_WIDTH;
        l--;
    }     
#ifdef _WINDOWS
    SystemUpdate();
#endif
}  
/******************************************************************************/
void DrawStringAbsBG(int x, int y, char *s, U8 c, U8 bg)
{
	int l=strlen(s);
    int x1=x,y1=y;
	while(l) {
        if(*s=='\n') {
            x1 = x;
        	y1+=CHAR_HEIGHT;
            if(!--l||*++s=='\0') break;
        }
    	AbChar(*s++,x1,y1,c,bg);
        x1+=CHAR_WIDTH;
        l--;
    }
#ifdef _WINDOWS
    SystemUpdate();
#endif
}
/******************************************************************************/
void DrawString(char *s,BOOL FIX)
{
	int l=strlen(s);

	while(l) {
        if(*s=='\n') {  
    		textCol = 0;
        	textRow++;
            if(!--l||*++s=='\0') break;
        }
    	DrawChar(*s++,FIX);
        l--;
    }
#ifdef _WINDOWS
    SystemUpdate();
#endif
}
/******************************************************************************/
void DrawAGIString(char *s)
{
	DrawString(FormatAGIString(s,msgBuf2),TRUE);
}
/******************************************************************************/
int GetStrNum(char **ps)
{
	char *s=*ps;
    int d =(*s++)-'0';
    if(ISNUM(*s)) {
    	d*=10;
        d+=(*s++)-'0';
    }
    if(ISNUM(*s)) {
    	d*=10;
        d+=(*s++)-'0';
    }
    *ps = s;
    return d;
}
/******************************************************************************/
char *FormatAGIString(char *s, char *out)
{
	int n;
    
    if(TEXT_MODE)
    	textColour &= 0x7F;

	while(*s) {
		if(*s != '%')
			*out++ = *s++;
		else {
        	s++;
			switch(*(s++)) {
            	//%vN	Value of var N
				case 'v':
                    sprintf(out,"%d",vars[GetStrNum(&s)]);
                    out+=strlen(out);
					break;
				//%wN	Nth word that the player typed in
				case 'w':
                	n = GetStrNum(&s)-1;
                    if(wordStrings[n]) {
                    	strcpy(out,wordStrings[n]);
                    	out+=strlen(out);
                    }
					break;
				//%sN	String N
				case 's':
                	FormatAGIString(strings[GetStrNum(&s)],out);
                    out+=strlen(out);
					break;
				//%mN	Message N
				case 'm':
                	FormatAGIString(GetMessage(curLog,GetStrNum(&s)),out);
                    out+=strlen(out);
					break;
				//%gN	Message N from logic 0
				case 'g':
                	FormatAGIString(GetMessage(log0,GetStrNum(&s)),out);
                    out+=strlen(out);
					break;
				//object name
				case 'o':
                    strcpy(out,(char*)objNames[GetStrNum(&s)]);
                    out+=strlen(out);
					break;
				default:		// not recognised
					*out++ = '%';
					s--;
			}
		}
	}
    *out = '\0';
	return msgBuf2;
}
/******************************************************************************/
char *WordWrap(char *s)
{
	char *pWord,*outStr=msgBuf;
    int lnLen,wLen;

    if(maxWidth<1)
    	maxWidth=MAX_MSG_WIDTH;
    msgWidth = 0;
    msgHeight = 0;
    lnLen 	= 0;
    if(*s)
    	msgHeight++;
    while(*s) {
    	pWord 	= s;
    	wLen	= 0;
		while(*s != '\0' && *s != ' ' && *s != '\n' && *s != '\r')
        	s++;
    	wLen = (int)(s-pWord);
        if(wLen&&*s=='\n'&&s[-1]==' ')
        	wLen--;
    	if(wLen+lnLen>=maxWidth) {
        	if(outStr[-1]==' ')
            	outStr[-1]='\n';
            else
            	*outStr++ = '\n';
            msgHeight++;
            lnLen = 0;
        	while(wLen>=maxWidth) {
        		msgWidth = maxWidth;
        		memcpy(outStr,pWord,maxWidth);
                wLen -= maxWidth;
            	outStr += maxWidth;
            	pWord  += maxWidth;
                *outStr++ = '\n';
                msgHeight++;
        	}
    	}
        if(wLen) {
        	memcpy(outStr,pWord,wLen);
        	outStr += wLen;
        }   
        lnLen += wLen+1;

        if(lnLen>msgWidth)
        	msgWidth=
            	(*s=='\0' || *s == ' ' ||
                *s == '\n' || *s == '\r')?
                	lnLen-1:lnLen;
        if(*s=='\n') {
        	msgHeight++;
            lnLen=0;
        }
    	if(*s)
        	*outStr++ = *s++;
	}
    *outStr = '\0';
    return msgBuf;
}
/******************************************************************************/
BOOL MessageBox(char *szMsg)
{
    int y, delay;
	char *s;
    BOOL ENTER_CLOSE=TRUE,DOFIX=FALSE;
	PUSH_TEXT_STYLE();

    szMsg=FormatAGIString(szMsg,msgBuf2);

	if(WINDOW_OPEN)
		cCloseWindow();

    textColour = 0xF0;

    s = WordWrap(szMsg);
    if(msgHeight>MAX_MSG_HEIGHT||msgWidth>maxWidth) {
    	ErrorMessage(ERR_PRINT);
    }
    ydiff			= 0;
	if(msgX<0||msgY<0) {
    	msgX = (40-msgWidth)>>1;
		msgY = ((21-msgHeight)>>1);
    	wndDraw.left	= (msgX-1)*(CHAR_WIDTH);
    	wndDraw.top		= (msgY-1)*CHAR_HEIGHT+2;
    	wndDraw.right	= (msgX+1+(msgWidth))*(CHAR_WIDTH);
    	wndDraw.bottom	= (msgY+msgHeight)*CHAR_HEIGHT+6;
    } else {
    	if(msgY+msgHeight>19) {
    		msgY = 20-msgHeight;
        	ydiff = -2;
        }
        wndDraw.left	= (msgX)*(CHAR_WIDTH);
        wndDraw.right	= (msgX+2+(msgWidth))*(CHAR_WIDTH);
        wndDraw.top		= (msgY)*CHAR_HEIGHT+2;    /*
        if(STATUS_VISIBLE) {
        	if(wndDraw.top -= Y_ADJUST_ST-(minRow?0:8)<2)
				wndDraw.top=2;
        } else {
        	if((wndDraw.top -= Y_ADJUST_CL-(minRow?0:8)) < 2)
				wndDraw.top=2;
        }                                                */
        wndDraw.bottom	= wndDraw.top+((1+msgHeight)*CHAR_HEIGHT)+(6+ydiff);
        msgX++;
        msgY++;
        //DOFIX=TRUE;
    }

    BoxNBorder(wndDraw.left,wndDraw.top,wndDraw.right,wndDraw.bottom,0x4F);

    textRow = msgY;
    y = msgHeight;
    while(y) {
    	textCol = msgX;
        while(*s&&*s!='\n') {
#ifdef FANCY_WINDOWS
        	DrawCharTrans(*s++,DOFIX,0);
#else
            DrawChar(*s++,DOFIX);
#endif
        }
        s++;
        textRow++;
    	y--;
    }

//#ifdef _WINDOWS
	SystemUpdate();
//#endif
    WINDOW_OPEN = TRUE;

   	POP_TEXT_STYLE();

	if(TestFlag(fPRINTMODE)) {
		ResetFlag(fPRINTMODE);
	} else {
#ifndef _WINDOWS
		if(vars[vPRINTDURATION]) {
			delay = vars[vPRINTDURATION];
			//Start the timer
            while(delay--) {
				REG_TM0CNT	= TIME_FREQUENcy256 | TIME_ENABLE;
				REG_TM0D	= 0;
				while(REG_TM0D<0x8000)
            		if((ENTER_CLOSE = CheckUserReply()) != -1) {
                    	delay = 0;
                        break;
                    }
				REG_TM0CNT	= 0;
            }
			vars[vPRINTDURATION] = 0;
        } else
#endif
			ENTER_CLOSE = WaitEnterEsc();

		cCloseWindow();
	}
    msgX			= -1;
    msgY			= -1;
    maxWidth		= -1;
    ydiff			= 0;
    return ENTER_CLOSE;
}
/******************************************************************************/
BOOL MessageBoxXY(char *szMsg, S8 row, S8 col, S8 width)
{
	BOOL RET;

    row--;
    if(row<0) row=0;

	msgY		= row;
	msgX		= col;
	maxWidth	= width;

	RET = MessageBox(szMsg);

	return RET;
}
/******************************************************************************/