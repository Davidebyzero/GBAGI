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
#ifndef _LOGIC_H
#define _LOGIC_H     
/*****************************************************************************/
typedef struct {
	U8 *code;
    U8 *messages;
	U8 num;
	U8 msgTotal;
} LOGIC;
/*****************************************************************************/

extern LOGIC *curLog,*log0;
extern BOOL IF_RESULT;
extern U16 logScan[256];
/*****************************************************************************/
char *GetMessage(LOGIC *log, int num);
void InitLogicSystem(void);
U8 *CallLogic(U8 num);
U8 *ExecuteLogic(LOGIC *log);

U8 *NewRoom(U8 num);

extern U8 *code;
/*****************************************************************************/
#endif
/*****************************************************************************/
