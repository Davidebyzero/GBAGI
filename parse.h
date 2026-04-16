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
#ifndef _PARSE_H
#define _PARSE_H       
#include "wingui.h"
/*****************************************************************************/
#define MAX_INPUT	10
extern U16 input[MAX_INPUT],inpos;
extern char *wordStrings[MAX_INPUT];
extern int wordCount;
/*****************************************************************************/

char *StripInput(char *sStart);
char *ParseInput(char *sStart);
int StrIsInt(char *string);
int StrToInt(char *string);
char *FindWord(char *szWord);
char *FindWordN(char *szWord);
void InitParseSystem(void);
void ExecuteInputDialog(BOOL CLEAR);
S16 wnInputProc(WND *w, U16 msg, U16 wParam, U32 lParam);
void ExecuteGetStringDialog(BOOL _GET_INT, U8 _dest, char *msg, int maxLen);
S16 wnGetStringProc(WND *w, U16 msg, U16 wParam, U32 lParam);
/*****************************************************************************/
#endif
/*****************************************************************************/
