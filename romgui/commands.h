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
#ifndef _COMMANDS_H
#define _COMMANDS_H
/*****************************************************************************/

#define MAX_TESTCMD		19
#define MAX_AGICMD		256//182

/*****************************************************************************/

/*****************************************************************************/
typedef struct {
	const char *name;
	const char *name2;
	U8 nParams;
	U8 pMask;
} AGICMD;

typedef struct {
	const char *name;
	const char *name2;
	U8 nParams;
	U8 pMask;
} AGITEST;

extern const AGITEST testCommands[MAX_TESTCMD];
extern const AGICMD agiCommands[MAX_AGICMD];
/*****************************************************************************/
#endif
/*****************************************************************************/
