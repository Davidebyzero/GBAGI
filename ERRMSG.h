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
#ifndef _ERRMSG_H
#define _ERRMSG_H
/*****************************************************************************/

enum {
	ERR_BAD_CMD,
	ERR_TEST_CMD,
	ERR_PIC_CODE,
	ERR_VOBJ_NUM,
	ERR_VOBJ_VIEW,
	ERR_VOBJ_LOOP,
	ERR_VOBJ_CEL,
	ERR_NO_VIEW,
	ERR_VOBJ_NO_CEL,
	ERR_PRINT,
	ERR_NOMENUSET,
	ERR_ADDMENUITEM,
	ERR_FINDMENUITEM,

    TOTAL_ERRORS
};

extern const char *Errors[TOTAL_ERRORS];    
/******************************************************************************/
int ErrorMessage(int num, ...);
void AGIExit(void);
/*****************************************************************************/
#endif
/*****************************************************************************/
