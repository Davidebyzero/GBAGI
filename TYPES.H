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
#ifndef _TYPES_H
#define _TYPES_H
/*****************************************************************************/
typedef unsigned char	U8;
typedef signed char		S8;
typedef unsigned short	U16;
typedef signed short	S16;
typedef unsigned long	U32;
typedef signed long		S32;
typedef int				BOOL;

typedef struct {
	U8 left,top,right,bottom;
} RECT8;

typedef struct {
	U16 left,top,right,bottom;
} RECT16;

typedef struct {
	S16 left,top,right,bottom;
} _RECT;

#define TRUE			1
#define FALSE			0

#define ISNUM(x) ((x>='0')&&(x<='9'))
/*****************************************************************************/
#endif
/*****************************************************************************/