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
#ifndef _SCREEN_H
#define _SCREEN_H
/*****************************************************************************/
//#define Y_ADJUST_ST			12//8//16
//#define Y_ADJUST_CL 		32//-8//4//4//0//8
extern S16 Y_ADJUST_CL;

#define SCREEN_WIDTH		240
#define SCREEN_HEIGHT		160
#define SCREEN_MAXX			239
#define SCREEN_MAXY			159
#define SCREEN_SIZE			(240*160)

#define PIC_WIDTH			160
#define PIC_HEIGHT			168
#define PIC_MAXX			(PIC_WIDTH-1)
#define PIC_MAXY			(PIC_HEIGHT-1)
#define PIC_SIZE			(PIC_WIDTH*PIC_HEIGHT)

#define PLAY_WIDTH			160
#define PLAY_SIZE           32000

#define PRI_WIDTH			80
#define PRI_HEIGHT			PIC_HEIGHT
#define PRI_MAXY  			(PRI_HEIGHT-1)
#define PRI_SIZE           	(PRI_WIDTH*PRI_HEIGHT)

#define GBA_MAXROW			19
#define GBA_MAXCOL			39
#define GBA_MAXY			151
#define GBASCR_SIZEOF		(19200)

#define VID_WIDE			120


#ifdef _WINDOWS
	extern U8 screenBuf[SCREEN_SIZE];   
	extern U8 pictureBuf[PIC_SIZE];
#else
	//extern U8 *pictureBuf;		// the active vis/pri buffer (160x168x(4bit+4bit))--1 byte = (pri/vis)
   	extern U16 *vidPtr;   
	extern U8 pictureBuf[PIC_SIZE] _EWRAM_;
#endif

extern _RECT port;
extern BOOL SHOW_VERSION;
/*****************************************************************************/
void ShakeScreen(int count);
void RotatePicBuf(void);
void RedrawScreenAll(void);
void RedrawScreen(void);
void UpdateGfx(void);
void DrawPlayArea(void);
void ShowPic(void);
void RefreshPri(void);
void PlotPix(int x, int y, U8 c1);
void PlotPixLO(int x, int y, U8 c1);
void PlotPixHI(int x, int y, U8 c1);   
void HilightPix(int x, int y);
void LolightPix(int x, int y);
void ShadowPix(int x, int y);
void HilightPix(int x, int y);
void ViewPlotPix(U16 x, U16 y, U8 cr);
void PicPlotPix(U8 x, U8 y, U8 c);
void ClearLine(int row, U8 c);
void ClearTextRect(int x1, int y1, int x2, int y2, U8 c);
void RenderUpdate(int x1, int y1, int x2, int y2);
void ClearScreen(U8 c);
void TransBox(int x1, int y1, int x2, int y2);
void ShadowBox(int x1, int y1, int x2, int y2);
void BitwiseBox(int x1, int y1, int x2, int y2, U8 and, U8 or);
void BoxNBorder(int x1, int y1, int x2, int y2, U8 c);
void RectFill(int x1, int y1, int x2, int y2, U8 c);
void RectFillX(int x1, int y1, int x2, int y2, U8 c);
void RectFillXHl(int x1, int y1, int x2, int y2, U8 c);

void ShadowBoxX(int x1, int y1, int x2, int y2);
void DrawScreenBlock(U8 *p,U16 *s,int w,int h);

/*****************************************************************************/
#endif
/*****************************************************************************/
