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
#include "picture.h"
#include "gamedata.h"
#include "errmsg.h"
#include "screen.h"
#include "views.h"
/*****************************************************************************/
const VFUNC PicCmds[11] = {
	PicCmd_VisEnable,
    PicCmd_VisDisable,
    PicCmd_PriEnable,
    PicCmd_PriDisable,
    PicCmd_YCorner,
    PicCmd_XCorner,
    PicCmd_AbsLine,
    PicCmd_RelLine,
    PicCmd_Fill,
    PicCmd_ChangePen,
    PicCmd_PlotPen
};
const U16 brushMap[8][16] = {
	{0x8000},
	{0x0E000, 0x0E000, 0x0E000},
	{0x7000, 0xF800, 0x0F800, 0x0F800, 0x7000},
	{0x3800, 0x7C00, 0x0FE00, 0x0FE00, 0x0FE00, 0x7C00, 0x3800},
	{0x1C00, 0x7F00, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x7F00, 0x1C00},
	{0x0E00, 0x3F80, 0x7FC0, 0x7FC0, 0x0FFE0, 0x0FFE0, 0x0FFE0, 0x7FC0, 0x7FC0, 0x3F80, 0x1F00, 0x0E00},
	{0x0F80, 0x3FE0, 0x7FF0, 0x7FF0, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x7FF0, 0x7FF0, 0x3FE0, 0x0F80},
	{0x07C0, 0x1FF0, 0x3FF8, 0x7FFC, 0x7FFC, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x7FFC, 0x7FFC, 0x3FF8, 0x1FF0, 0x07C0}
};
/*****************************************************************************/
U8 *picData, *picPtr, pcode;
U8 overlays[MAX_OVERLAYS],*pOverlay;
U8	picX1, picY1, oldX1, oldY1, picX2, picY2;
U8 col, fillCol, fillMask, plotMask;
U8	picStack[4096],*pStk,
	left, right, prevLeft, prevRight, fillDir, prevFillDir, nLeft, nRight;
BOOL toggle, prevToggle;
U16 penMode;
S16 penX, penY;
U16 penFill;

/*****************************************************************************/
// Set up the initial picture variables
void InitPicSystem(BOOL FULL)
{
	picX1			= 0;
	picY1			= 0;
	picX2			= 0;
	picY2			= 0;
	penX 			= 0;
	penY 			= 0;
	penMode 		= 0;
	penFill 		= 0;
	plotMask		= 0;
	col				= 0xFF;

	if(FULL) {
		picData		= NULL;
    	pOverlay	= overlays;
    }
}


/*****************************************************************************/
// Draw a new picture clearing any previous one
void DrawPic(U8 num)
{
//FILE *f;
//int i;
//num = 11;
	picNum	= num;
	picData	= (U8*)picDir[num]+5;

	EraseBlitLists();
	RenderPic(PIC_FULLDRAW);
	DrawBlitLists();

	PIC_VISIBLE = FALSE;

//cShowPriScreen();
}

/*****************************************************************************/
// Draw another picture over an existing one
void OverlayPic(U8 num)
{
	picNum = num;
	picData = (U8*)picDir[num]+5;

	EraseBlitLists();
	RenderPic(PIC_OVERLAY);
	DrawBlitLists();
	UpdateBlitLists();

	PIC_VISIBLE = FALSE;

    if(pOverlay<overlays+MAX_OVERLAYS)
    	*pOverlay++ = num;
}

/*****************************************************************************/
// Draws the actual picture from the data to the buffer
void RenderPic(BOOL CLEAR)
{
	InitPicSystem(FALSE);

	if(CLEAR) {
		memset(pictureBuf, 0x4F, sizeof(pictureBuf));
    	pOverlay	= overlays;
   		pPView		= pViews;
    }

    picPtr = picData;
	pcode = *picPtr++;
	while(pcode >= 0xF0 && pcode <= 0xFA)
    	PicCmds[pcode&0x0F]();
}


/*###########################################################################*/

/*****************************************************************************/
// retreives the next bytes of picture data as a x and y coordinates and
// formats them accordingly
BOOL ReadCoords(U8 *x, U8 *y)
{
	if(!ReadCoord(x,PIC_MAXX))
		return FALSE;
	return(ReadCoord(y,PIC_MAXY));
}
/*****************************************************************************/
// retreives the next byte of picture data as an x or y coordinate and formats
// it accordingly
BOOL ReadCoord(U8 *c, int max)
{
	if( (*c = pcode = *picPtr++) >= 0xF0 )
		return FALSE;
	if(*c > max)
		*c = max;
	return TRUE;
}
/*###########################################################################*/


/*****************************************************************************/
// PIC CMD 0xF0: Turn On Visual Draw and Select Colour
void PicCmd_VisEnable()
{
	plotMask	|= 0x0F;
	col			= (col & 0xF0) | (*picPtr++);
	pcode		= *picPtr++;
}

/*****************************************************************************/
// PIC CMD 0xF1: Turn Off Visual Draw
void PicCmd_VisDisable()
{
	plotMask	&= 0xF0;
	col			|= 0x0F;
	pcode 		= *picPtr++;
}

/*****************************************************************************/
// PIC CMD 0xF2: Turn On Priority Draw and Select Colour
void PicCmd_PriEnable()
{
	col			= ((*picPtr++ << 4)&0xF0)|(col & 0x0F);
	plotMask	|= 0xF0;
	pcode 		= *picPtr++;
}
/*****************************************************************************/
// PIC CMD 0xF3: Turn Off Priority Draw
void PicCmd_PriDisable()
{
	plotMask	&= 0x0F;
	col			|= 0xF0;
	pcode 		= *picPtr++;
}
/*****************************************************************************/
// PIC CMD 0xF4: Y Corner
void PicCmd_YCorner()
{
	// fetch two bytes for the absolute coords
	if(ReadCoords(&picX1, &picY1)) {
    	// it'll draw at least one pixel...
		PicBufPlot();
        // draw the vertical line, then horizontal...
    	while(DrawYCorner())
        	if(!DrawXCorner()) return;
	}
}
/*****************************************************************************/
// PIC CMD 0xF5: X Corner
void PicCmd_XCorner()
{
	// fetch two bytes for the absolute coords
	if(ReadCoords(&picX1, &picY1)) {
    	// it'll draw at least one pixel...
		PicBufPlot();
       	// opposite of the y corner
    	while(DrawXCorner())
        	if(!DrawYCorner()) return;
	}
}
/*****************************************************************************/
// PIC CMD: 0xF6: Absolute line
void PicCmd_AbsLine()
{
	// grab two bytes specifying the absolute coords
	if(ReadCoords(&picX1, &picY1)) {
    	// all lines no matter how small will draw at least one pixel
		PicBufPlot();
        // grab two more bytes specifying the next absolute coords
		while(ReadCoords(&picX2, &picY2))
			LineDraw();
	}
}
/*****************************************************************************/
// PIC CMD 0xF7: Relative Line
void PicCmd_RelLine()
{
	U8 x, y;

    // grab two bytes with the absolute coords
	if(ReadCoords(&picX1, &picY1)) {
		// draw a dot if it doesn't end up being long enough for a line to actually be generated
		PicBufPlot();
		while( (pcode = *picPtr++) < 0xF0 ) {
    		// get a relative signed four bit x difference
    	    // it's stored as aBBB
    	    //	- a: 1=negative, 0:positive
    	    //	- b: 0-7 value
			if((x = (pcode&0x80)?picX1-((pcode>>4)&7):picX1+((pcode>>4)&7)) > PIC_MAXX)
				x = PIC_MAXX;
    		// get a relative signed four bit y difference
			if((y = (pcode&0x08)?picY1-(pcode&7):picY1+(pcode&7)) > PIC_MAXY)
				y = PIC_MAXY;
			picX2 = x;
			picY2 = y;
			LineDraw();
        }
	}
}
/*****************************************************************************/
// PIC CMD 0xF8: Flood Fill
void PicCmd_Fill()
{
	// just filling absolute coords until next instruction
	while(ReadCoords(&picX1, &picY1))
		PicBufFloodFill(picY1, picX1);
}
/*****************************************************************************/
// PIC CMD 0xF9: Change Pen Attributes
void PicCmd_ChangePen()
{
	penMode	= *picPtr++;
	pcode	= *picPtr++;
}
/*****************************************************************************/
// PIC CMD: 0xFA: Pen Plot
void PicCmd_PlotPen()
{
	U8 xx, yy;
    U16 *p;
	int	x, width, endX, endY, pfill, penSize, penSize2;

	for(;;) {
		if(penMode & PEN_SPLATTER) {
			if((pcode = *picPtr++) >= 0xF0)
				return;
			penFill = pcode;
		}
		if(!ReadCoords(&xx, &yy))
			return;
		if((penX = (xx<<1) - (penSize = penMode & 7)) < 0)
    		penX = 0;
		if((PIC_WIDTH<<1)-(penSize2=(penSize<<1)) < penX)
			penX = (PIC_WIDTH<<1) - penSize2;
		endX = (penX >>= 1);

		if((penY = yy - (penMode & 7)) < 0)
    		penY = 0;
		if(PIC_MAXY-penSize2 < penY)
			penY = PIC_MAXY - penSize2;
		endY = penY+penSize2+1;

		width	= (penSize2+1) << 1;
		p		= (U16*)brushMap[penSize];
		pfill	= penFill | 1;
		do {
			x = 0;
			do {
				if(((penMode&PEN_RECTANGLE) || ((0x8000>>(x>>1))&*p))) {
                  	if((!(penMode&PEN_SPLATTER))) {
                  		picY1 = (U8)penY;
                        picX1 = (U8)penX;
                        PicBufPlot();
                  	} else {
						int pfillbit = pfill &1;
						pfill >>= 1;
						if(pfillbit)
							pfill ^= 0xB8;
						if( (pfill & 3) == 1 ) {
                  			picY1 = (U8)penY;
							picX1 = (U8)penX;
							PicBufPlot();
                    	}
                  	}
				}
				penX++;
			} while ((x+=4) <= width);
			penX = endX;
            p++;
		} while(++penY < endY);
	}
}
/*****************************************************************************/

/*###########################################################################*/

/*****************************************************************************/
// draw a line on the picture buffer
void LineDraw()
{
	int	wide, lineLen, x = picX1, y = picY1,
    	dirX, dirY, xDiff, yDiff, dcX = 0, dcY = 0;

    if(x == picX2) {
		VLineDraw();
		return;
	}
	if(y == picY2) {
		HLineDraw();
		return;
	}

	if((xDiff = picX2 - picX1) < 0) {
		dirX	= -1;
		xDiff	= 0-xDiff;
	} else
    	dirX	= 1;

	if((yDiff = picY2 - picY1) < 0) {
		dirY	= -1;
		yDiff	= 0-yDiff;
	} else
    	dirY	= 1;

	if(xDiff < yDiff)
		dcX	= (lineLen = wide = yDiff)>>1;
	else
    	dcY	= (lineLen = wide = xDiff)>>1;

	do {
		if((dcX += xDiff) >= wide) {
			dcX	-= wide;
			x	+= dirX;
		}
		if((dcY += yDiff) >= wide) {
			dcY	-= wide;
			y	+= dirY;
		}
		picX1 = x;
		picY1 = y;
		PicBufPlot();
	} while(--lineLen);
}

/*****************************************************************************/
// draw a straight line on the horizontal axis
void HLineDraw()
{
	int endX = picX2, x1, x2, len;
	U8 *b;

	if(picX1 < picX2) {
		x1		= picX1;
		x2		= picX2;
    } else {
		x1		= picX2;
		x2		= picX1;
		picX1	= x1;
		picX2	= x2;
	}

	b  = MAKE_PICBUF_PTR(picX1,picY1);
	len = x2 - x1 + 1;
	do
		*b++ = (*b | plotMask) & col;
    while(--len);

	picX1	= endX;
}
/*****************************************************************************/
// draws a x corner, a straight line across the horizontal axis
BOOL DrawXCorner()
{
	U8 pos, finX, finY;

	if(!ReadCoord(&pos,PIC_MAXX))
		return FALSE;

	picX2	= pos;
	picY2	= picY1;

	finX	= picX2;
	finY	= picY2;
	HLineDraw();
	picY1	= finY;
	picX1	= finX;

	return TRUE;
}
/*****************************************************************************/
// draw a straight line on the vertical axis
void VLineDraw()
{
	int endY=picY2, y1, y2, len;
	U8 *b;

	if(picY1 < picY2) {
		y1		= picY1;
		y2		= picY2;
	} else  {
		y1		= picY2;
		y2		= picY1;
		picY1	= y1;
		picY2	= y2;
	}

	b = MAKE_PICBUF_PTR(picX1,picY1);
	len = y2 - y1 + 1;
	do {
		*b = (*b | plotMask) & col;
		b += PIC_WIDTH;
		y1++;
	} while(--len);

	picY1 = endY;
}
/*****************************************************************************/
// draws a y corner, a straight line across the vertical axis
BOOL DrawYCorner()
{
	U8 coord, finX, finY;

	if(!ReadCoord(&coord,PIC_MAXY))
		return FALSE;

	picX2	= picX1;
	picY2	= coord;

	finX	= picX2;
	finY	= picY2;
	VLineDraw();
	picX1	= finX;
	picY1	= finY;

	return TRUE;
}

/*****************************************************************************/
// Plot a pixel on the picture buffer
void PicBufPlot()
{
	U8 *pBuf = MAKE_PICBUF_PTR(picX1,picY1);
	*pBuf = (*pBuf | plotMask) & col;
}
/*****************************************************************************/
// Flood fill a region on the picture buffer
void PicBufFloodFill(U8 ypos, U8 xpos)
{
	int i=7;
/*
FILE *f = fopen("e:\\test.raw","wb");
int j;
for(j=0;j<160*168;j++)
	fputc(pictureBuf[j]&0xF0,f);
fclose(f);
RenderUpdate(0,0,PIC_MAXX,PIC_MAXY);
WaitEnterEsc();   */

    do
    	picStack[--i] = 0xFF;
    while(i);

	fillCol	= 0x4F;
	left		= PIC_WIDTH+1;
	right		= 0;
	fillDir 	= 1;
	toggle 		= FALSE;

	if(plotMask&0x0F) {
		if((col&0x0F) == 0x0F)
        	return;
		fillMask = 0x0F;
	} else {
		if( (!(plotMask & 0xF0)) || ((col&0xF0) == 0x40) )
        	return;
		fillMask = 0xF0;
	}

	if( (fillCol &= fillMask) != (*MAKE_PICBUF_PTR(xpos,ypos) & fillMask) )
		return;

    pStk = picStack+7;
    FillCallback(MAKE_PICBUF_PTR(xpos,ypos));
}
/*****************************************************************************/
void FillPush()
{
		if((right > prevRight) || ((right == prevRight) && (left != prevLeft))) {
        	toggle	= FALSE;
            oldX1	= prevRight;
		} else if(right == prevRight) {
			if(toggle)
            	return;
			toggle	= TRUE;
			oldX1	= right;
		} else {
			toggle	= FALSE;
			oldX1	= right;
		}
		FILLSTACK_PUSH(prevToggle);
		FILLSTACK_PUSH(fillDir);
		FILLSTACK_PUSH(prevFillDir);
		FILLSTACK_PUSH(oldY1);
		FILLSTACK_PUSH(oldX1);
		FILLSTACK_PUSH(prevRight);
		FILLSTACK_PUSH(prevLeft);
}
/*****************************************************************************/
void FillCallback(U8 *b)
{
	U8 *b2, *b3;
	U8 origCol;
    int i, lastY1;

	prevRight	= right;
	prevLeft	= left;
	prevToggle	= toggle;
	oldX1		= picX1;

	origCol	= *(b2 = b);

	i = picX1+1;
	do {
		*b-- = (origCol | plotMask) & col;
		origCol = *b;
	} while( ( (origCol & fillMask) == fillCol) && --i );

	i 		= PIC_MAXX - picX1;
	left	= (picX1 -= (b2 - ++b));

	b3 		= ++b2;
	b2		= b;
	b 		= b3;

	while(i-- && (((origCol = *b) & fillMask) == fillCol))
		*b++ = (origCol|plotMask) & col;

	right =  left + (b - b2) - 1;

	if(prevLeft <= PIC_MAXX)
    	FillPush();

	prevFillDir	= fillDir;
	oldY1		= picY1;
	picY1		+= fillDir;

	for(;;) {
		if(picY1 <= PIC_MAXY) {
       		for(;;) {
				if((*(b = MAKE_PICBUF_PTR(picX1,picY1)) & fillMask) == fillCol) {
                	FillCallback(b);
                    return;
                }
				if(	(toggle) || (fillDir == prevFillDir) || (picX1 < nLeft) || (picX1 > nRight) ) {
					if(picX1 >= right) break;
             		picX1++;
             		continue;
            	}
				if(nRight < right)
					if((picX1 = nRight+1) < right) {
						picX1++;
						continue;
					}
            	break;
            }
        }
		if( (fillDir == prevFillDir) && (!toggle) ) {
			fillDir	= -fillDir;
			picX1	= left;
			picY1	= lastY1 = oldY1;
		} else {
			FILLSTACK_POP(left);
			FILLSTACK_POP(right);
			FILLSTACK_POP(picX1);
			FILLSTACK_POP(picY1);
			FILLSTACK_POP(prevFillDir);
			FILLSTACK_POP(fillDir);
			FILLSTACK_POP(toggle);

			if((lastY1 = picY1) == 0xFF)
				return;
			oldY1 = picY1;
		}
		nLeft	= FILLSTACK_PEEK(-1);
		nRight	= FILLSTACK_PEEK(-2);
		picY1	= lastY1 + fillDir;
	}
}
/*****************************************************************************/

