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
#ifndef _MENU_H
#define _MENU_H
/*****************************************************************************/

#define MI_ENABLED		0x01

typedef struct _MENUITEM {
	struct _MENUITEM *prev,*next;
	char *name;
	U8 properties;
	U8 controller;
	U8 row;
} MENUITEM;

typedef struct _MENU {
	struct _MENU *prev,*next;
	MENUITEM *items;
	char *name;
	U8 column;
    U8 width;
    U8 height;
} MENU;

extern MENU *menu,*menuLast;
extern BOOL MENU_SELECTABLE;
/*****************************************************************************/
void InitMenuSystem(void);
void FreeMenuSystem(void);

void SetMenu(char *caption);
void SetMenuItem(char *caption,U8 ctl);
void SubmitMenu(void);
void ToggleMenuItem(U8 ctl,BOOL ENABLE);
void EnableAllMenuItems(void);
void MenuInput(void);

MENUITEM *FindMenuItem(U8 ctl);
MENUITEM *GetLastItem(MENUITEM *miStart);
MENUITEM *GetFirstItem(MENUITEM *miStart);

void DrawMenuBar(void);
void ClearMenuItems(void);
void DrawMenuItems(MENUITEM *miActive);
BOOL IsMenuSeparator(MENUITEM *mi);
/*****************************************************************************/
#endif
/*****************************************************************************/