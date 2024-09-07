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
#include "system.h"
#include "agimain.h"
#include "system.h" 
#include "gamedata.h"
#include "wingui.h"
#include "picture.h"
#include "screen.h"
#include "text.h"
/*****************************************************************************/

/*****************************************************************************/
#include "extra/splashdata.c"
#include "extra/splashlogodata.c"
void LoadPicBufSplash(void);    

char szerr[] = "- ERROR:  No AGI Game embedded in ROM! -";

int main()
{
    int x,y,i;
	if(!SystemInit())
    	return 1;
	//DrawStringAbs(0,0,"System Initialized...",0xF7);
	//DrawStringAbs(0,8,"Initializing Desktop Advance...",0xF7);
	//DrawStringAbs(0,16,"Initializing Game Data...",0xF7);
    for(;;) {
    	InitWinGUISystem(0);

        LoadPicBufSplash();

        PIC_VISIBLE=TRUE;
        SHOW_VERSION = TRUE;
        ShowPic();
		if(!GameDataInit()) {
			DrawStringAbs(0,0,szerr,0x00);
			DrawStringAbs(2,2,szerr,0x00);
			DrawStringAbs(1,1,szerr,0xCC);
    		return 2;
    	}
        SHOW_VERSION = FALSE;
		//DrawStringAbs(0,24,"Executing AGI...",0xF7);
		//RunGUI1();
        QUIT_FLAG = FALSE;
    	if(!AGIInit(FALSE))
    		return 3;
    	AGIMain();
    	FreeWinGUISystem();
    }
}
/*****************************************************************************/
void LoadPicBufSplash()
{                         /*
	//memset(pictureBuf, 0x05, sizeof(pictureBuf));
    FILE *f=fopen("e:\\gbagi\\_work\\4.gcp","rb");
   	U16 l = fgetc(f)+(fgetc(f)<<8);
    U8 *pb = pictureBuf;
    int x,y;
    while(l--) {
    	U8 c = fgetc(f);
     	int cnt = (c>>4);
        c&=0xF;
        while(cnt-- >= 0) {
            *pb++ = c;
        }
    }     */     /*
    for(y=0;y<168;y++)
    	for(x=0;x<160;x++) {
    		U8 c = fgetc(f);
            if(c==1) *pb++ = (x^y)&1?1:8;
        	else *pb++ = c;
        }
               */ /*
        i=0;
        for(y=0;y<168;y++) {
        	for(x=0;x<160;x++) {
         		pictureBuf[i++] = (x^y)&1?0x45:0x4E;
            }
        }  */
   	int w=54,h=60;
    int x,y;
    U8 *ptrbase = pictureBuf+(10*PIC_WIDTH)+(2);
    U8 *bmp = splashData;
    int i=0;
    for(y=0;y<168;y++) {
       	for(x=0;x<160;x++) {
       		pictureBuf[i++] = (x^y)&3?0:1;
        }
    }
    for(y=0;y<h;y++) {
    	U8 *pb = ptrbase+(y*(PIC_WIDTH*2));
    	for(x=0;x<w;x+=2) {
    		U8 c = *bmp++;
     		if((c&0xF)!=4) {
            	pb[0] =
     			pb[PIC_WIDTH] = c&0xF;
            }
     		if((c>>4)!=4) {
     			pb[1] =
     			pb[PIC_WIDTH+1] = c>>4;
            }
            pb+=2;
        }
    }
   	w=98;
    h=63;
    ptrbase = pictureBuf+(15*PIC_WIDTH)+(59);
    bmp = splashLogoData;
    for(y=0;y<h;y++) {
    	U8 *pb = ptrbase+(y*(PIC_WIDTH*2));
    	for(x=0;x<w;x+=2) {
    		U8 c = *bmp++;
     		if((c&0xF)!=1) {
            	pb[0] =
     			pb[PIC_WIDTH] = c&0xF;
            }
     		if((c>>4)!=1) {
     			pb[1] =
     			pb[PIC_WIDTH+1] = c>>4;
            }
            pb+=2;
        }
    }
}
/*****************************************************************************/

