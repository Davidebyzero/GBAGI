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

#define INTERUPT_C

#include "gbagi.h"

extern fp intr_main;
//these are my function definitios to let the c compiler know what I am talking about

void VBLANK(void) ;
void HBLANK(void) ;
void DMA0(void) ;
void DMA1(void) ;
void DMA2(void) ;
void DMA3(void) ;
void VCOUNT(void) ;
void TIMER0(void) ;
void TIMER1(void) ;
void TIMER2(void) ;
void TIMER3(void) ;
void KEYBOARD(void) ;
void CART(void) ;
void COMUNICATION(void) ;


//here is a function to enable interupts.  If it is VBLANK or HBLANK it sets REG_DISPSTAT to the apropriate values
//all others you will have to do on your own.

void EnableInterupts(U16 interupts)
{
	REG_IME = 0;  //probably not necessary but not a good idea to change interupt registers when an interupt is ocuring


	if(interupts == INT_VBLANK)
		REG_DISPSTAT |= 0x8;

	if(interupts == INT_HBLANK)
		REG_DISPSTAT |= 0x10;
	

	REG_IE |= interupts;

	REG_IME = 1;

}

void DissableInterupts(U16 interupts)
{
	REG_IE &= ~interupts;

	if(interupts == INT_ALL) //this is where that ALL comes in
		REG_IME = 0;  //disable all interupts;
}




//here is our table of function pointers.  I added the definition of fp to gba.h because I figure it would be usefull
//to have elsewhere
//typedef void (*fp)(void);   this is the definition you will find in gba.h.  It is just delaring fp to represent a pointer
//to a function


//here is our table of functions.  Once interupts are enabled in crt0.s you will not be able to link your code if this table is not pressent.
//the interupts must be listed in that order.

fp IntrTable[]  = 
{
	VBLANK,
	HBLANK,
	VCOUNT,
	TIMER0,
	TIMER1,
	TIMER2,
	TIMER3,
	COMUNICATION,
	DMA0,
	DMA1,
	DMA2,
	DMA3,
	KEYBOARD,
	CART
};



//all our interupt functions empty for now.

void VBLANK()
{	
	
	REG_IF |= INT_VBLANK;
		
}
void HBLANK(void)
{
	
	REG_IF |= INT_HBLANK;
}
void DMA0(void)
{
	
	REG_IF |= INT_DMA0;

}
void DMA1(void)
{
	
	REG_IF |= INT_DMA1;

}
void DMA2(void)
{
	
	REG_IF |= INT_DMA2;

}
void DMA3(void)
{
	REG_IF |= INT_DMA3;

}
void VCOUNT(void)
{
	REG_IF |= INT_VCOUNT;

}
void TIMER0(void)
{
	REG_IF |= INT_TIMER0;

}
void TIMER1(void)
{
	REG_IF |= INT_TIMER1;

}

int volumesX[16]={
	 0, 8, 9, 9,
	10,10,11,11,
	12,12,13,13,
	14,14,15,15
};
int volumesY[16]={
	0,1,2,2,
	3,3,4,4,
	5,5,6,6,
	7,7,8,8
};
int volumes2[16]={
	0,1,2,2,
	3,3,4,4,
	5,5,6,6,
	7,7,8,8
};
int volumes3[16]={
	0,3,3,3,
	2,2,2,2,
	4,4,4,4,
	1,1,1,1
};
U16 sweeptime=0,sweepdir=0,sweepshifts=0,envinit=15,envdir=1,envsteptime=7,
	waveduty=3,loopmode=0;
S16 freq=0,nfreq=0,totalwa; 

void TIMER2(void)
{
	U16 len;
	int i;
	int *volumes=volumesX;
	
	if(TestFlag(fSOUND)&&sndBuf) {/*
		if(!sndWaits[3]--) {
			if((len = pSnds[3][0]+(pSnds[3][1]<<8))!=0xFFFF) {
				sndWaits[3] = len-1;
				envinit=volumes2[((pSnds[3][4]&0xF))^0xF];
				if(envinit) {
					REG_SOUND4CNT_L=(volumes[envinit]<<12)+(envdir<<11)+(envsteptime<<8);
					REG_SOUND4CNT_H=SOUND4INIT+(loopmode<<14)+(7-(pSnds[3][4]&3));
				}
				volumes=volumesY;
				pSnds[3]+=5;
			} else {
				REG_SOUND4CNT_L=0;
				REG_SOUND4CNT_H=SOUND4INIT+0;
				REG_SOUND4CNT_H=0;
			}	
		}*/
		if(pSnds[0]&&!sndWaits[0]--) {
				if((len = pSnds[0][0]+(pSnds[0][1]<<8))==0xFFFF) {
					StopSound();
					freq=1;
                    pSnds[0]=NULL;
				} else {
					sndWaits[0]=len-1;
					len =0;
					nfreq = (((U16)pSnds[0][2] & 0x3F) << 4) | (U16)(pSnds[0][3] & 0xF);
					freq = 0x7FF-(nfreq);
					envinit=volumes[((pSnds[0][4]&0xF))^0xF];
					REG_SOUND1CNT_L=0;//(sweeptime<<4)+(sweepdir<<3)+sweepshifts;
					REG_SOUND1CNT_H=(envinit<<12)+(envdir<<11)+(envsteptime<<8)+(waveduty<<6);
					REG_SOUND1CNT_X=SOUND1INIT+(loopmode<<14)+freq;
					pSnds[0]+=5;
				}
				//RectFill(2, 156-24, 4, 156, 0);
				//	RectFill(2, 156-(sndWaits[0]>>2), 4, 156, envinit);
		}
		if(pSnds[1]&&!sndWaits[1]--) {
				if((len = pSnds[1][0]+(pSnds[1][1]<<8))==0xFFFF) {
					REG_SOUND2CNT_L=0;
					REG_SOUND2CNT_H=SOUND2INIT+0;
					REG_SOUND2CNT_H=0;
					freq=1;
                    pSnds[1]=NULL;
				} else {
					sndWaits[1]=len-1;
					len =0;
					nfreq = (((U16)pSnds[1][2] & 0x3F) << 4) | (U16)(pSnds[1][3] & 0xF);
					freq = 0x7FF-(nfreq);
					envinit=volumes[((pSnds[1][4]&0xF))^0xF];
					REG_SOUND2CNT_L=(envinit<<12)+(envdir<<11)+(envsteptime<<8)+(waveduty<<6);
					REG_SOUND2CNT_H=SOUND2INIT+(loopmode<<14)+freq;
					pSnds[1]+=5;
				}
				//RectFill(6, 156-24, 8, 156, 0);
				//RectFill(6, 156-(sndWaits[1]>>2), 8, 156, envinit);
		}
		if(pSnds[2]&&!sndWaits[2]--) {
				if((len = pSnds[2][0]+(pSnds[2][1]<<8))==0xFFFF) {
					REG_SOUND3CNT_L=0;
					REG_SOUND3CNT_H=SOUND3INIT+0;
					REG_SOUND3CNT_H=0;
					REG_SOUND3CNT_X=0;
                    pSnds[2]=NULL;
				} else {
					sndWaits[2]=len-1;
					len =0;
					nfreq = (((U16)pSnds[2][2] & 0x3F) << 4) | (U16)(pSnds[2][3] & 0xF);
					freq = 0x7FF-(nfreq);
					envinit = volumes3[((pSnds[2][4]&0xF))^0xF];
					REG_SOUND3CNT_L=           SOUND3SETBANK1+SOUND3BANK32; // select bank 0 for writing (bank 1 for playing)
					for (i=0; i<4; i++)
						(&REG_WAVE_RAM0)[i] = i&1 ? 0 : 0xFFFFFFFF;
					REG_SOUND3CNT_L=SOUND3PLAY+SOUND3SETBANK0+SOUND3BANK32; // select bank 0 for playing
					REG_SOUND3CNT_H=(envinit<<13)+0;
					REG_SOUND3CNT_X=SOUND3INIT+SOUND3PLAYLOOP+freq;
					pSnds[2]+=5;
				}
		}
	} else {
		if(!TestFlag(fSOUND))
			StopSound();
	}
	REG_IF |= INT_TIMER2;
}
void TIMER3(void)
{	
	
    if(++vars[vSECONDS]>=60) {
    	vars[vSECONDS]=0;
        if(++vars[vMINUTES]>=60) {
    		vars[vMINUTES]=0;
        	if(++vars[vHOURS]>=60) {
    			vars[vHOURS]=0;
                vars[vDAYS]++;
            }
        }
    }  
	REG_IF |= INT_TIMER3;

}
void KEYBOARD(void)
{
	REG_IF |= INT_TIMER3;
}
void CART(void)
{
	REG_IF |= INT_CART;

}
void COMUNICATION(void)
{
	REG_IF |= INT_COMUNICATION;

}


