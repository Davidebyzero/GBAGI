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

/******************************************************************************/
#include <tchar.h>
#include "makerom.h"
/******************************************************************************/

VERLIST verlist[] = {
	{
		_T("Early version 2"),
	    0,
	    	0x02, 0x0272
	},
	{
		_T("Common version 2"),
	    ENCRYPT_OBJ,
	    	0x02, 0x0917
	},
	{
	  	_T("Amiga version 2 equiv"),
	    ENCRYPT_OBJ|AMIGA,
	    	0x02, 0x0917
	},
	{
	  	_T("Early version 3"),
	    SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ,
	    	0x03, 0x2086
	},
	{
	  	_T("Common version 3"),
	    SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ,
	    	0x03, 0x2149
	},
	{
	  	_T("Amiga version 3 equiv"),
	    SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ|AMIGA,
	    	0x03, 0x2149
	},
	{
		NULL
	}
};
/******************************************************************************/
GAMEINFO games[] = {
	{&verlist[1], "",    "King's Quest 1",            _T("D:\\agigames\\King's Quest 1 (AGI)\\")},
	{&verlist[1], "",    "King's Quest 2",            _T("D:\\agigames\\King's Quest 2\\")},
	{&verlist[1], "",    "King's Quest 3",            _T("D:\\agigames\\KingsQuest3\\")},
	{&verlist[3], "KQ4", "King's Quest 4",            _T("D:\\agigames\\King's Quest 4 (AGI)\\")},
	{&verlist[4], "DM",  "King's Quest 4 Demo",       _T("D:\\agigames\\King's Quest 4 (Demo)\\")},
	{&verlist[1], "",    "Leisure Suit Larry",        _T("D:\\agigames\\Leisure Suit Larry 1\\")},
	{&verlist[4], "MH",  "Manhunter 1:New York",      _T("D:\\agigames\\Manhunter 1 - New York\\")},
	{&verlist[4], "MH2", "Manhunter 2:San Francisco", _T("D:\\agigames\\Manhunter 2 - San Francisco\\")},
	{&verlist[1], "",    "Police Quest",              _T("D:\\agigames\\Police Quest 1 (AGI)\\")},   
	{&verlist[1], "",    "Space Quest 1",             _T("D:\\agigames\\Space Quest 1 (AGI)\\")},
	{&verlist[1], "",    "Space Quest 2",             _T("D:\\agigames\\Space Quest 2\\")},    
	{&verlist[4], "GR",  "Gold Rush!",                _T("D:\\agigames\\Goldrush\\")},
	{&verlist[1], "",    "The Black Cauldron",        _T("D:\\agigames\\The Black Cauldron\\")},
	{&verlist[0], "",    "Donald Duck's Playground",  _T("D:\\agigames\\Donald Duck's Playground\\")},
	{&verlist[1], "",    "Mixed Up Mother Goose",     _T("D:\\agigames\\Mixed Up Mother Goose\\")},
	{&verlist[4], "DM",  "Demo Pack #4",              _T("D:\\agigames\\demopac4\\")},
	{NULL,"","<Other>",_T("")},
	{NULL,NULL,NULL}
};
/******************************************************************************/
