// gba.h by eloist 

#ifndef GBA_HEADER
#define GBA_HEADER

#define FIXED s32
#define PI 3.14159
#define RADIAN(n) 		(((float)n)/(float)180*PI)

#define OAMmem  		((volatile U32*)0x7000000)
#define VideoBuffer 	((volatile U16*)0x6000000)
#define VideoBuffer1 	((volatile U16*)0x6000000)
#define VideoBuffer2 	((volatile U16*)0x600A000)
#define OAMData			((volatile U16*)0x6010000)
#define BGPaletteMem 	((volatile U16*)0x5000000)
#define OBJPaletteMem 	((volatile U16*)0x5000200)
		
#define REG_INTERUPT   *(volatile U32*)0x3007FFC
#define REG_DISPCNT    *(volatile U32*)0x4000000
#define REG_DISPCNT_L  *(volatile U16*)0x4000000
#define REG_DISPCNT_H  *(volatile U16*)0x4000002
#define REG_DISPSTAT   *(volatile U16*)0x4000004
#define REG_VCOUNT     *(volatile U16*)0x4000006
#define REG_BG0CNT     *(volatile U16*)0x4000008
#define REG_BG1CNT     *(volatile U16*)0x400000A
#define REG_BG2CNT     *(volatile U16*)0x400000C
#define REG_BG3CNT     *(volatile U16*)0x400000E
#define REG_BG0HOFS    *(volatile U16*)0x4000010
#define REG_BG0VOFS    *(volatile U16*)0x4000012
#define REG_BG1HOFS    *(volatile U16*)0x4000014
#define REG_BG1VOFS    *(volatile U16*)0x4000016
#define REG_BG2HOFS    *(volatile U16*)0x4000018
#define REG_BG2VOFS    *(volatile U16*)0x400001A
#define REG_BG3HOFS    *(volatile U16*)0x400001C
#define REG_BG3VOFS    *(volatile U16*)0x400001E
#define REG_BG2PA      *(volatile U16*)0x4000020
#define REG_BG2PB      *(volatile U16*)0x4000022
#define REG_BG2PC      *(volatile U16*)0x4000024
#define REG_BG2PD      *(volatile U16*)0x4000026
#define REG_BG2X       *(volatile U32*)0x4000028
#define REG_BG2X_L     *(volatile U16*)0x4000028
#define REG_BG2X_H     *(volatile U16*)0x400002A
#define REG_BG2Y       *(volatile U32*)0x400002C
#define REG_BG2Y_L     *(volatile U16*)0x400002C
#define REG_BG2Y_H     *(volatile U16*)0x400002E
#define REG_BG3PA      *(volatile U16*)0x4000030
#define REG_BG3PB      *(volatile U16*)0x4000032
#define REG_BG3PC      *(volatile U16*)0x4000034
#define REG_BG3PD      *(volatile U16*)0x4000036
#define REG_BG3X       *(volatile U32*)0x4000038
#define REG_BG3X_L     *(volatile U16*)0x4000038
#define REG_BG3X_H     *(volatile U16*)0x400003A
#define REG_BG3Y       *(volatile U32*)0x400003C
#define REG_BG3Y_L     *(volatile U16*)0x400003C
#define REG_BG3Y_H     *(volatile U16*)0x400003E
#define REG_WIN0H      *(volatile U16*)0x4000040
#define REG_WIN1H      *(volatile U16*)0x4000042
#define REG_WIN0V      *(volatile U16*)0x4000044
#define REG_WIN1V      *(volatile U16*)0x4000046
#define REG_WININ      *(volatile U16*)0x4000048
#define REG_WINOUT     *(volatile U16*)0x400004A
#define REG_MOSAIC     *(volatile U32*)0x400004C
#define REG_MOSAIC_L   *(volatile U32*)0x400004C
#define REG_MOSAIC_H   *(volatile U32*)0x400004E
#define REG_BLDMOD     *(volatile U16*)0x4000050
#define REG_COLEV      *(volatile U16*)0x4000052
#define REG_COLEY      *(volatile U16*)0x4000054
#define REG_SG10       *(volatile U32*)0x4000060
#define REG_SG10_L     *(volatile U16*)0x4000060
#define REG_SG10_H     *(volatile U16*)0x4000062
#define REG_SG11       *(volatile U16*)0x4000064
#define REG_SG20       *(volatile U16*)0x4000068
#define REG_SG21       *(volatile U16*)0x400006C
#define REG_SG30       *(volatile U32*)0x4000070
#define REG_SG30_L     *(volatile U16*)0x4000070
#define REG_SG30_H     *(volatile U16*)0x4000072
#define REG_SG31       *(volatile U16*)0x4000074
#define REG_SG40       *(volatile U16*)0x4000078
#define REG_SG41       *(volatile U16*)0x400007C
#define REG_SGCNT0     *(volatile U32*)0x4000080
#define REG_SGCNT0_L   *(volatile U16*)0x4000080
#define REG_SGCNT0_H   *(volatile U16*)0x4000082
#define REG_SGCNT1     *(volatile U16*)0x4000084
#define REG_SGBIAS     *(volatile U16*)0x4000088
#define REG_SGWR0      *(volatile U32*)0x4000090
#define REG_SGWR0_L    *(volatile U16*)0x4000090
#define REG_SGWR0_H    *(volatile U16*)0x4000092
#define REG_SGWR1      *(volatile U32*)0x4000094
#define REG_SGWR1_L    *(volatile U16*)0x4000094
#define REG_SGWR1_H    *(volatile U16*)0x4000096
#define REG_SGWR2      *(volatile U32*)0x4000098
#define REG_SGWR2_L    *(volatile U16*)0x4000098
#define REG_SGWR2_H    *(volatile U16*)0x400009A
#define REG_SGWR3      *(volatile U32*)0x400009C
#define REG_SGWR3_L    *(volatile U16*)0x400009C
#define REG_SGWR3_H    *(volatile U16*)0x400009E
#define REG_SGFIF0A    *(volatile U32*)0x40000A0
#define REG_SGFIFOA_L  *(volatile U16*)0x40000A0
#define REG_SGFIFOA_H  *(volatile U16*)0x40000A2
#define REG_SGFIFOB    *(volatile U32*)0x40000A4
#define REG_SGFIFOB_L  *(volatile U16*)0x40000A4
#define REG_SGFIFOB_H  *(volatile U16*)0x40000A6
#define REG_DM0SAD     *(volatile U32*)0x40000B0
#define REG_DM0SAD_L   *(volatile U16*)0x40000B0
#define REG_DM0SAD_H   *(volatile U16*)0x40000B2
#define REG_DM0DAD     *(volatile U32*)0x40000B4
#define REG_DM0DAD_L   *(volatile U16*)0x40000B4
#define REG_DM0DAD_H   *(volatile U16*)0x40000B6
#define REG_DM0CNT     *(volatile U32*)0x40000B8
#define REG_DM0CNT_L   *(volatile U16*)0x40000B8
#define REG_DM0CNT_H   *(volatile U16*)0x40000BA
#define REG_DM1SAD     *(volatile U32*)0x40000BC
#define REG_DM1SAD_L   *(volatile U16*)0x40000BC
#define REG_DM1SAD_H   *(volatile U16*)0x40000BE
#define REG_DM1DAD     *(volatile U32*)0x40000C0
#define REG_DM1DAD_L   *(volatile U16*)0x40000C0
#define REG_DM1DAD_H   *(volatile U16*)0x40000C2
#define REG_DM1CNT     *(volatile U32*)0x40000C4
#define REG_DM1CNT_L   *(volatile U16*)0x40000C4
#define REG_DM1CNT_H   *(volatile U16*)0x40000C6
#define REG_DM2SAD     *(volatile U32*)0x40000C8
#define REG_DM2SAD_L   *(volatile U16*)0x40000C8
#define REG_DM2SAD_H   *(volatile U16*)0x40000CA
#define REG_DM2DAD     *(volatile U32*)0x40000CC
#define REG_DM2DAD_L   *(volatile U16*)0x40000CC
#define REG_DM2DAD_H   *(volatile U16*)0x40000CE
#define REG_DM2CNT     *(volatile U32*)0x40000D0
#define REG_DM2CNT_L   *(volatile U16*)0x40000D0
#define REG_DM2CNT_H   *(volatile U16*)0x40000D2
#define REG_DM3SAD     *(volatile U32*)0x40000D4
#define REG_DM3SAD_L   *(volatile U16*)0x40000D4
#define REG_DM3SAD_H   *(volatile U16*)0x40000D6
#define REG_DM3DAD     *(volatile U32*)0x40000D8
#define REG_DM3DAD_L   *(volatile U16*)0x40000D8
#define REG_DM3DAD_H   *(volatile U16*)0x40000DA
#define REG_DM3CNT     *(volatile U32*)0x40000DC
#define REG_DM3CNT_L   *(volatile U16*)0x40000DC
#define REG_DM3CNT_H   *(volatile U16*)0x40000DE
#define REG_TM0D       *(volatile U16*)0x4000100
#define REG_TM0CNT     *(volatile U16*)0x4000102
#define REG_TM1D       *(volatile U16*)0x4000104
#define REG_TM1CNT     *(volatile U16*)0x4000106
#define REG_TM2D       *(volatile U16*)0x4000108
#define REG_TM2CNT     *(volatile U16*)0x400010A
#define REG_TM3D       *(volatile U16*)0x400010C
#define REG_TM3CNT     *(volatile U16*)0x400010E
#define REG_SCD0       *(volatile U16*)0x4000120
#define REG_SCD1       *(volatile U16*)0x4000122
#define REG_SCD2       *(volatile U16*)0x4000124
#define REG_SCD3       *(volatile U16*)0x4000126
#define REG_SCCNT      *(volatile U32*)0x4000128
#define REG_SCCNT_L    *(volatile U16*)0x4000128
#define REG_SCCNT_H    *(volatile U16*)0x400012A
#define REG_P1         *(volatile U16*)0x4000130
#define REG_P1CNT      *(volatile U16*)0x4000132
#define REG_R          *(volatile U16*)0x4000134
#define REG_HS_CTRL    *(volatile U16*)0x4000140
#define REG_JOYRE      *(volatile U32*)0x4000150
#define REG_JOYRE_L    *(volatile U16*)0x4000150
#define REG_JOYRE_H    *(volatile U16*)0x4000152
#define REG_JOYTR      *(volatile U32*)0x4000154
#define REG_JOYTR_L    *(volatile U16*)0x4000154
#define REG_JOYTR_H    *(volatile U16*)0x4000156
#define REG_JSTAT      *(volatile U32*)0x4000158
#define REG_JSTAT_L    *(volatile U16*)0x4000158
#define REG_JSTAT_H    *(volatile U16*)0x400015A
#define REG_IE         *(volatile U16*)0x4000200
#define REG_IF         *(volatile U16*)0x4000202
#define REG_WSCNT      *(volatile U16*)0x4000204
#define REG_IME        *(volatile U16*)0x4000208
#define REG_PAUSE      *(volatile U16*)0x4000300


#include "keypad.h"
#include "screenmode.h"

#ifdef _WINDOWS
	#define _EWRAM_ ;
#else
	#define _EWRAM_  __attribute__ ((section (".ewram")))
#endif


typedef void (*fp)(void);

//a couple of functions to make turing interrupts on and off easier

void EnableInterupts(volatile U16 interupts);
void DissableInterupts(volatile U16 interupts);

#define INT_VBLANK 0x0001
#define INT_HBLANK 0x0002 
#define INT_VCOUNT 0x0004 //you can set the display to generate an interrupt when it reaches a particular line on the screen
#define INT_TIMER0 0x0008
#define INT_TIMER1 0x0010
#define INT_TIMER2 0x0020 
#define INT_TIMER3 0x0040
#define INT_COMUNICATION 0x0080 //serial communication interupt
#define INT_DMA0 0x0100
#define INT_DMA1 0x0200
#define INT_DMA2 0x0400
#define INT_DMA3 0x0800
#define INT_KEYBOARD 0x1000
#define INT_CART 0x2000 //the cart can actually generate an interupt
#define INT_ALL 0x4000 //this is just a flag we can set to allow the my function to enable or disable all interrupts. Doesn't actually correspond to a bit in REG_IE 

#define TIME_FREQUENcySYSTEM 0x0
#define TIME_FREQUENcy64 0x1
#define TIME_FREQUENcy256 0x2
#define TIME_FREQUENcy1024 0x3
#define TIME_OVERFLOW 0x4
#define TIME_ENABLE 0x80
#define TIME_IRQ_ENABLE 0x40

#define REG_SOUND1CNT   *(volatile U32*)0x4000060	//sound 1
#define REG_SOUND1CNT_L *(volatile U16*)0x4000060	//
#define REG_SOUND1CNT_H *(volatile U16*)0x4000062	//
#define REG_SOUND1CNT_X *(volatile U16*)0x4000064	//

#define REG_SOUND2CNT_L *(volatile U16*)0x4000068		//sound 2 lenght & wave duty
#define REG_SOUND2CNT_H *(volatile U16*)0x400006C		//sound 2 frequency+loop+reset

#define REG_SG30       *(volatile U32*)0x4000070		//???
#define REG_SOUND3CNT  *(volatile U32*)0x4000070		//???
#define REG_SG30_L     *(volatile U16*)0x4000070		//???
#define REG_SOUND3CNT_L *(volatile U16*)0x4000070	//???
#define REG_SG30_H     *(volatile U16*)0x4000072		//???
#define REG_SOUND3CNT_H *(volatile U16*)0x4000072	//???
#define REG_SG31       *(volatile U16*)0x4000074		//???
#define REG_SOUND3CNT_X *(volatile U16*)0x4000074	//???

#define REG_SOUND4CNT_L *(volatile U16*)0x4000078		//???
#define REG_SOUND4CNT_H *(volatile U16*)0x400007C		//???

#define REG_SGCNT0     *(volatile U32*)0x4000080		
#define REG_SGCNT0_L   *(volatile U16*)0x4000080		
#define REG_SOUNDCNT   *(volatile U32*)0x4000080
#define REG_SOUNDCNT_L *(volatile U16*)0x4000080		//DMG sound control

#define REG_SGCNT0_H   *(volatile U16*)0x4000082		
#define REG_SOUNDCNT_H *(volatile U16*)0x4000082		//Direct sound control

#define REG_SGCNT1     *(volatile U16*)0x4000084		
#define REG_SOUNDCNT_X *(volatile U16*)0x4000084	    //Extended sound control

#define REG_SGBIAS     *(volatile U16*)0x4000088		
#define REG_SOUNDBIAS  *(volatile U16*)0x4000088		//bit rate+sound bias

#define REG_WAVE_RAM0  *(volatile U32*)0x4000090		//???
#define REG_SGWR0_L    *(volatile U16*)0x4000090		//???
#define REG_SGWR0_H    *(volatile U16*)0x4000092		//???
#define REG_WAVE_RAM1  *(volatile U32*)0x4000094		//???
#define REG_SGWR1_L    *(volatile U16*)0x4000094		//???
#define REG_SGWR1_H    *(volatile U16*)0x4000096		//???
#define REG_WAVE_RAM2  *(volatile U32*)0x4000098		//???
#define REG_SGWR2_L    *(volatile U16*)0x4000098		//???
#define REG_SGWR2_H    *(volatile U16*)0x400009A		//???
#define REG_WAVE_RAM3  *(volatile U32*)0x400009C		//???
#define REG_SGWR3_L    *(volatile U16*)0x400009C		//???
#define REG_SGWR3_H    *(volatile U16*)0x400009E		//???


////////////////sound channels/////////////
#define SOUNDINIT			0x8000	// makes the sound restart
#define SOUNDDUTY87			0x0000	//87.5% wave duty
#define SOUNDDUTY75			0x0040	//75% wave duty
#define SOUNDDUTY50			0x0080	//50% wave duty
#define SOUNDDUTY25			0x00C0	//25% wave duty

#define SOUND1PLAYONCE		0x4000	// play sound once
#define SOUND1PLAYLOOP		0x0000	// play sound looped
#define SOUND1INIT			0x8000	// makes the sound restart
#define SOUND1SWEEPSHIFTS(n)	n	// number of sweep shifts (0-7)
#define SOUND1SWEEPINC		0x0000	// sweep add (freq increase)
#define SOUND1SWEEPDEC		0x0008	// sweep dec (freq decrese)
#define SOUND1SWEEPTIME(n)	(n<<4)	// time of sweep (0-7)
#define SOUND1ENVSTEPS(n)	(n<<8)	// envelope steps (0-7)
#define SOUND1ENVINC		0x0800	// envellope increase
#define SOUND1ENVDEC		0x0000	// envellope decrease
#define SOUND1ENVINIT(n)	(n<<12) // initial envelope volume (0-15)


#define SOUND2PLAYONCE		0x4000	// play sound once
#define SOUND2PLAYLOOP		0x0000	// play sound looped
#define SOUND2INIT			0x8000	// makes the sound restart
#define SOUND2ENVSTEPS(n)	(n<<8)	// envelope steps (0-7)
#define SOUND2ENVINC		0x0800	// envellope increase
#define SOUND2ENVDEC		0x0000	// envellope decrease
#define SOUND2ENVINIT(n)	(n<<12) // initial envelope volume (0-15)



#define SOUND3BANK32		0x0000	// Use two banks of 32 steps each
#define SOUND3BANK64		0x0020	// Use one bank of 64 steps
#define SOUND3SETBANK0		0x0000	// Bank to play 0 or 1 (non set bank is written to)
#define SOUND3SETBANK1		0x0040
#define SOUND3PLAY			0x0080	// Output sound

#define SOUND3OUTPUT0		0x0000	// Mute output
#define SOUND3OUTPUT1		0x2000	// Output unmodified
#define SOUND3OUTPUT12		0x4000	// Output 1/2 
#define SOUND3OUTPUT14		0x6000	// Output 1/4 
#define SOUND3OUTPUT34		0x8000  // Output 3/4

#define SOUND3PLAYONCE		0x4000	// Play sound once
#define SOUND3PLAYLOOP		0x0000	// Play sound looped
#define SOUND3INIT			0x8000	// Makes the sound restart


#define SOUND4PLAYONCE		0x4000	// play sound once
#define SOUND4PLAYLOOP		0x0000	// play sound looped
#define SOUND4INIT			0x8000	// makes the sound restart
#define SOUND4ENVSTEPS(n)	(n<<8)	// envelope steps (0-7)
#define SOUND4ENVINC		0x0800	// envellope increase
#define SOUND4ENVDEC		0x0000	// envellope decrease
#define SOUND4ENVINIT(n)	(n<<12) // initial envelope volume (0-15)


#define SOUND4STEPS7		0x0004
#define SOUND4STEPS15		0x0000
#define SOUND4PLAYONCE		0x4000
#define SOUND4PLAYLOOP		0x0000
#define SOUND4INIT			0x8000

#define REG_SGFIFOA    *(volatile U32*)0x40000A0		//???
#define REG_SGFIFOA_L  *(volatile U16*)0x40000A0		//???
#define REG_SGFIFOA_H  *(volatile U16*)0x40000A2		//???
#define REG_SGFIFOB    *(volatile U32*)0x40000A4		//???
#define REG_SGFIFOB_L  *(volatile U16*)0x40000A4		//???
#define REG_SGFIFOB_H  *(volatile U16*)0x40000A6		//???
#define REG_DMA0SAD     *(volatile U32*)0x40000B0	//DMA0 Source Address
#define REG_DMA0SAD_L   *(volatile U16*)0x40000B0	//DMA0 Source Address Low Value
#define REG_DMA0SAD_H   *(volatile U16*)0x40000B2	//DMA0 Source Address High Value
#define REG_DMA0DAD     *(volatile U32*)0x40000B4	//DMA0 Destination Address
#define REG_DMA0DAD_L   *(volatile U16*)0x40000B4	//DMA0 Destination Address Low Value
#define REG_DMA0DAD_H   *(volatile U16*)0x40000B6	//DMA0 Destination Address High Value
#define REG_DMA0CNT     *(volatile U32*)0x40000B8	//DMA0 Control (Amount)
#define REG_DMA0CNT_L   *(volatile U16*)0x40000B8	//DMA0 Control Low Value
#define REG_DMA0CNT_H   *(volatile U16*)0x40000BA	//DMA0 Control High Value

#define REG_DMA1SAD     *(volatile U32*)0x40000BC	//DMA1 Source Address
#define REG_DMA1SAD_L   *(volatile U16*)0x40000BC	//DMA1 Source Address Low Value
#define REG_DMA1SAD_H   *(volatile U16*)0x40000BE	//DMA1 Source Address High Value
#define REG_DMA1DAD     *(volatile U32*)0x40000C0	//DMA1 Desination Address
#define REG_DMA1DAD_L   *(volatile U16*)0x40000C0	//DMA1 Destination Address Low Value
#define REG_DMA1DAD_H   *(volatile U16*)0x40000C2	//DMA1 Destination Address High Value
#define REG_DMA1CNT     *(volatile U32*)0x40000C4	//DMA1 Control (Amount)
#define REG_DMA1CNT_L   *(volatile U16*)0x40000C4	//DMA1 Control Low Value
#define REG_DMA1CNT_H   *(volatile U16*)0x40000C6	//DMA1 Control High Value

#define REG_DMA2SAD     *(volatile U32*)0x40000C8	//DMA2 Source Address
#define REG_DMA2SAD_L   *(volatile U16*)0x40000C8	//DMA2 Source Address Low Value
#define REG_DMA2SAD_H   *(volatile U16*)0x40000CA	//DMA2 Source Address High Value
#define REG_DMA2DAD     *(volatile U32*)0x40000CC	//DMA2 Destination Address
#define REG_DMA2DAD_L   *(volatile U16*)0x40000CC	//DMA2 Destination Address Low Value
#define REG_DMA2DAD_H   *(volatile U16*)0x40000CE	//DMA2 Destination Address High Value
#define REG_DMA2CNT     *(volatile U32*)0x40000D0	//DMA2 Control (Amount)
#define REG_DMA2CNT_L   *(volatile U16*)0x40000D0	//DMA2 Control Low Value
#define REG_DMA2CNT_H   *(volatile U16*)0x40000D2	//DMA2 Control High Value

#define REG_DMA3SAD     *(volatile U32*)0x40000D4	//DMA3 Source Address
#define REG_DMA3SAD_L   *(volatile U16*)0x40000D4	//DMA3 Source Address Low Value
#define REG_DMA3SAD_H   *(volatile U16*)0x40000D6	//DMA3 Source Address High Value
#define REG_DMA3DAD     *(volatile U32*)0x40000D8	//DMA3 Destination Address
#define REG_DMA3DAD_L   *(volatile U16*)0x40000D8	//DMA3 Destination Address Low Value
#define REG_DMA3DAD_H   *(volatile U16*)0x40000DA	//DMA3 Destination Address High Value
#define REG_DMA3CNT     *(volatile U32*)0x40000DC	//DMA3 Control (Amount)
#define REG_DMA3CNT_L   *(volatile U16*)0x40000DC	//DMA3 Control Low Value
#define REG_DMA3CNT_H   *(volatile U16*)0x40000DE	//DMA3 Control High Value


#endif
