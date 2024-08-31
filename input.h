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
#ifndef _INPUT_H
#define _INPUT_H
/*****************************************************************************/
typedef struct {
	U16 btn;
	U8 state;
    U8 kbstate;
    U16 kbkey, kbrow, kbcol;
} BTNSTATE;

#define BTN_IDLE		0
#define BTN_PRESS		1
#define BTN_RELEASE		2
#define BTN_HOLD		3
#define BTN_INJECTED	4
                              
#define EV_DIRECTION	1
#define EV_ASCII		2

extern BTNSTATE btnstate;

typedef struct {
	S16 type;
	U16 data;
} EVENT;

typedef struct {
	U16 symbol;
	U16 value;
} KEY;

#define KEY_TABREV		0x8001
#define KEY_TABFWD		0x8002
#define KEY_START		0x8003
#define KEY_SELECT		0x8004
#define KEY_RESET		0x8005

#define KEY_A			(0x1C)
#define KEY_B 			(0x32)
#define KEY_C 			(0x2E)
#define KEY_D 			(0x20)
#define KEY_E 			(0x12)
#define KEY_F 			(0x21)
#define KEY_G 			(0x22)
#define KEY_H 			(0x23)
#define KEY_I 			(0x17)
#define KEY_J 			(0x24)
#define KEY_K 			(0x25)
#define KEY_L 			(0x26)
#define KEY_M 			(0x32)
#define KEY_N 			(0x31)
#define KEY_O 			(0x18)
#define KEY_P 			(0x19)
#define KEY_Q 			(0x10)
#define KEY_R 			(0x13)
#define KEY_S 			(0x1F)
#define KEY_T 			(0x14)
#define KEY_U 			(0x16)
#define KEY_V 			(0x2F)
#define KEY_W 			(0x11)
#define KEY_X 			(0x2D)
#define KEY_Y 			(0x15)
#define KEY_Z 			(0x2C)
#define KEY_1 			(0x02)
#define KEY_2 			(0x03)
#define KEY_3 			(0x04)
#define KEY_4 			(0x05)
#define KEY_5 			(0x06)
#define KEY_6 			(0x07)
#define KEY_7 			(0x08)
#define KEY_8 			(0x09)
#define KEY_9 			(0x0A)
#define KEY_0 			(0x0B)
#define KEY_DASH		(0x0C)		/* -_ */
#define KEY_EQUAL		(0x0D)		/* =+ */
#define KEY_LBRACKET	(0x1A)		/* [{ */
#define KEY_RBRACKET	(0x1B)		/* ]} */
#define KEY_SEMICOLON	(0x27)		/* ;: */
#define KEY_RQUOTE		(0x28)		/* '" */
#define KEY_LQUOTE		(0x29)		/* `~ */
#define KEY_PERIOD		(0x33)		/* .> */
#define KEY_COMMA		(0x34)		/* ,< */
#define KEY_SLASH		(0x35)		/* /? */
#define KEY_BACKSLASH	(0x2B)		/* \| */
#define KEY_F1 			(0x3B)
#define KEY_F2 			(0x3C)
#define KEY_F3 			(0x3D)
#define KEY_F4 			(0x3E)
#define KEY_F5 			(0x3F)
#define KEY_F6 			(0x40)
#define KEY_F7 			(0x41)
#define KEY_F8 			(0x42)
#define KEY_F9 			(0x43)
#define KEY_F10 		(0x44)
#define KEY_ESC 		(0x1B)
#define KEY_BACKSPACE 	(0x0E)
#define KEY_TAB 		(0x0F)
#define KEY_SHIFTTAB	(0x8F)
#define KEY_ENTER 		(0x0D)
#define KEY_CONTROL 	(0x1D)
#define KEY_CTRL	 	(0x1D)
#define KEY_SHIFT 		(0x2A)
#define KEY_LSHIFT 		(0x2A)
#define KEY_RSHIFT 		(0x36)
#define KEY_PRTSC 		(0x37)
#define KEY_ALT 		(0x38)
#define KEY_SPACE 		(0x39)
#define KEY_CAPSLOCK 	(0x3A)
#define KEY_NUMLOCK 	(0x45)
#define KEY_SCROLLLOCK 	(0x46)
#define KEY_HOME 		(0x47)
#define KEY_UP 			(0x48)
#define KEY_PGUP 		(0x49)
#define KEY_MINUS 		(0x4A)
#define KEY_LEFT 		(0x4B)
#define KEY_CENTER 		(0x4C)
#define KEY_RIGHT 		(0x4D)
#define KEY_PLUS 		(0x4E)
#define KEY_END 		(0x4F)
#define KEY_DOWN 		(0x50)
#define KEY_PGDOWN 		(0x51)
#define KEY_INS 		(0x52)
#define KEY_DEL 		(0x53)

#define KEY_NUMPAD0		(0x60)
#define KEY_NUMPAD1        0x61
#define KEY_NUMPAD2        0x62
#define KEY_NUMPAD3        0x63
#define KEY_NUMPAD4        0x64
#define KEY_NUMPAD5        0x65
#define KEY_NUMPAD6        0x66
#define KEY_NUMPAD7        0x67
#define KEY_NUMPAD8        0x68
#define KEY_NUMPAD9        0x69
#define KEY_MULTIPLY       0x6A
#define KEY_ADD            0x6B
#define KEY_SEPARATOR      0x6C
#define KEY_SUBTRACT       0x6D
#define KEY_DECIMAL        0x6E
#define KEY_DIVIDE         0x6F


#define DIR_MAX 			8

enum {
	DIR_N = 1,
	DIR_NE,
	DIR_E,
	DIR_SE,
	DIR_S,
	DIR_SW,
	DIR_W,
	DIR_NW
};
/*****************************************************************************/
void DoDelayNPoll(void);

void Delay(int amt);
void PollInput(void);
EVENT *ReadEvent(void);
int PollKey(void);
int WaitForKey(void);
int CheckUserReply(void);
BOOL WaitEnterEsc(void);
EVENT *WaitForEvent(void);
/*****************************************************************************/
#endif
/*****************************************************************************/
