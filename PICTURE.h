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
#ifndef _PICTURE_H
#define _PICTURE_H
/*****************************************************************************/

#define VIS_DRAW			1
#define PRI_DRAW			2

typedef void (*PICFUNC)(void);

#define MAKE_PICBUF_PTR(x,y) \
	((U8*)&(pictureBuf)[((y)*PIC_WIDTH) + (x)])

#define FILLSTACK_PUSH(x)\
	*pStk++ = x;
#define FILLSTACK_POP(x)\
	x = *--pStk;
#define FILLSTACK_PEEK(x)\
	pStk[x]

#define PRI_CONTROL		(0x10)
#define PRI_SIGNAL		(0x20)
#define PRI_WATER		(0x30)
#define FLAG_CONTROL	(0x01)
#define FLAG_SIGNAL		(0x02)
#define FLAG_WATER		(0x04)

#define PIC_CORNER_X	0
#define PIC_CORNER_Y	1

#define PEN_RECTANGLE	0x10
#define PEN_SPLATTER	0x20

typedef void (*VFUNC)(void);
/*****************************************************************************/
#define PIC_FULLDRAW	1
#define PIC_OVERLAY		0

void InitPicSystem(BOOL FULL);
void DrawPic(U8 num);
void OverlayPic(U8 num);
void RenderPic(BOOL CLEAR);
void PicCmd_VisEnable(void);
void PicCmd_VisDisable(void);
void PicCmd_PriEnable(void);
void PicCmd_PriDisable(void);
void PicCmd_PlotPen(void);
void PicCmd_ChangePen(void);
void PicCmd_AbsLine(void);
void PicCmd_XCorner(void);
void PicCmd_YCorner(void);
BOOL DrawXCorner(void);
BOOL DrawYCorner(void);
void PicCmd_RelLine(void);
void PicCmd_Fill(void);
BOOL ReadCoords(U8 *x, U8 *y);
BOOL ReadCoord(U8 *c, int max);
void LineDraw(void);
void HLineDraw(void);
void VLineDraw(void);
void PicBufPlot(void);
void PicBufFloodFill(U8 ypos, U8 xpos);
void FillPush(void);
BOOL FillCallback(U8 *b);
/*****************************************************************************/
#define MAX_OVERLAYS 16
extern U8 overlays[MAX_OVERLAYS],*pOverlay;
/*****************************************************************************/
#endif
/*****************************************************************************/
