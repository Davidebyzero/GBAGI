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
#include "makerom.h"
/******************************************************************************/

VERLIST verlist[] = {
	{
		"Early version 2",
	    0,
	    	0x02, 0x0272
	},
	{
		"Common version 2",
	    ENCRYPT_OBJ,
	    	0x02, 0x0917
	},
	{
	  	"Amiga version 2 equiv",
	    ENCRYPT_OBJ|AMIGA,
	    	0x02, 0x0917
	},
	{
	  	"Early version 3",
	    SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ,
	    	0x03, 0x2086
	},
	{
	  	"Common version 3",
	    SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ,
	    	0x03, 0x2149
	},
	{
	  	"Amiga version 3 equiv",
	    SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ|AMIGA,
	    	0x03, 0x2149
	},
	{
		NULL
	}
};
/******************************************************************************/
GAMEINFO games[] = {
	{&verlist[1],	"",		"King's Quest 1", 				"D:\\agigames\\King's Quest 1 (AGI)\\"},
	{&verlist[1],	"",		"King's Quest 2", 				"D:\\agigames\\King's Quest 2\\"},
	{&verlist[1],	"",		"King's Quest 3",				"D:\\agigames\\KingsQuest3\\"},
	{&verlist[3],	"KQ4",	"King's Quest 4", 				"D:\\agigames\\King's Quest 4 (AGI)\\"},
	{&verlist[4],	"DM", 	"King's Quest 4 Demo", 			"D:\\agigames\\King's Quest 4 (Demo)\\"},
	{&verlist[1],	"",		"Leisure Suit Larry",			"D:\\agigames\\Leisure Suit Larry 1\\"},
	{&verlist[4],	"MH",	"Manhunter 1:New York", 		"D:\\agigames\\Manhunter 1 - New York\\"},
	{&verlist[4],	"MH2",	"Manhunter 2:San Francisco",	"D:\\agigames\\Manhunter 2 - San Francisco\\"},
	{&verlist[1], "",		"Police Quest", 				"D:\\agigames\\Police Quest 1 (AGI)\\"},   
	{&verlist[1], "",		"Space Quest 1", 				"D:\\agigames\\Space Quest 1 (AGI)\\"},
	{&verlist[1], "",		"Space Quest 2", 				"D:\\agigames\\Space Quest 2\\"},    
	{&verlist[4],	"GR",	"Gold Rush!",					"D:\\agigames\\Goldrush\\"},
	{&verlist[1],	"",		"The Black Cauldron", 			"D:\\agigames\\The Black Cauldron\\"},
	{&verlist[0],	"",   	"Donald Duck's Playground", 	"D:\\agigames\\Donald Duck's Playground\\"},
	{&verlist[1],	"",   	"Mixed Up Mother Goose", 		"D:\\agigames\\Mixed Up Mother Goose\\"},
	{&verlist[4],	"DM", 	"Demo Pack #4", 				"D:\\agigames\\demopac4\\"},
	{NULL,"","<Other>",""},
	{NULL,NULL,NULL}
};
/******************************************************************************/
