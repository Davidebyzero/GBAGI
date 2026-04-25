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
#include "gbagi.h"
#include "menu.h"   
#include "input.h"
#include "text.h"
#include "errmsg.h"
#include "screen.h"
#include "status.h"  
#include "wingui.h"
#include "system.h"
#include "views.h"
#include "commands.h"
/*****************************************************************************/
MENU *menu,*menuLast,*activeMenu;
MENUITEM *lastItem,*activeItem;
RECT8 menuRect;
static char walkModeMenuCaption[] = "Input: Classic Mode";
static char audioModeMenuCaption[] = "SFX Voice: Original";
static char musicBackendMenuCaption[] = "Music Backend: Live Tandy";
static char characterBoostMenuCaption[] = "Move Boost: On";
static char audioBoostMenuCaption[] = "Audio Boost: On";
static char larrySpeedHackMenuCaption[] = "Larry Room Boost: On";
static char comboBoostMenuCaption[] = "Combo Boost: On";

static enum audio_mode GetNextAudioMode(enum audio_mode mode)
{
	switch(mode) {
		case AUDIO_ORIGINAL:
			return AUDIO_SEMI_TANDY;
		case AUDIO_SEMI_TANDY:
			return AUDIO_TANDY;
		default:
			return AUDIO_ORIGINAL;
	}
}

static enum audio_music_backend GetNextAudioMusicBackend(enum audio_music_backend mode)
{
	return (mode == AUDIO_BACKEND_TANDY_LIVE) ? AUDIO_BACKEND_TANDY_HYBRID : AUDIO_BACKEND_TANDY_LIVE;
}

#define MB_SIZEOF	((sizeof(MENU)*12)+(sizeof(MENUITEM)*36))
#define MB_END		(menuBuf+MB_SIZEOF)
U8 menuBuf[MB_SIZEOF],*mbPtr;
BOOL MENU_SELECTABLE;
/*****************************************************************************/
static void UpdateWalkModeMenuCaption(void)
{
	strcpy(walkModeMenuCaption, WALK_HOLD ? "Input: Classic Mode" : "Input: Dpad Mode");
}
/*****************************************************************************/
static void UpdateAudioModeMenuCaption(void)
{
	switch(GetAudioMode()) {
		case AUDIO_ORIGINAL:
			strcpy(audioModeMenuCaption, "SFX Voice: Original");
			break;
		case AUDIO_SEMI_TANDY:
			strcpy(audioModeMenuCaption, "SFX Voice: Semi-Tandy");
			break;
		default:
			strcpy(audioModeMenuCaption, "SFX Voice: Tandy");
			break;
	}
}
/*****************************************************************************/
static void UpdateMusicBackendMenuCaption(void)
{
	strcpy(
		musicBackendMenuCaption,
		(GetAudioMusicBackend() == AUDIO_BACKEND_TANDY_HYBRID) ? "Music Backend: Streamed PCM" : "Music Backend: Live Tandy"
	);
}
/*****************************************************************************/
static void UpdateCharacterBoostMenuCaption(void)
{
	strcpy(
		characterBoostMenuCaption,
		IsCharacterBoostEnabled() ? "Move Boost: On" : "Move Boost: Off"
	);
}
/*****************************************************************************/
static void UpdateAudioBoostMenuCaption(void)
{
	strcpy(
		audioBoostMenuCaption,
		IsAudioBoostEnabled() ? "Audio Boost: On" : "Audio Boost: Off"
	);
}
/*****************************************************************************/
static void UpdateLarrySpeedHackMenuCaption(void)
{
	strcpy(
		larrySpeedHackMenuCaption,
		IsLarryMusicSpeedHackEnabled() ? "Larry Room Boost: On" : "Larry Room Boost: Off"
	);
}
/*****************************************************************************/
static void UpdateComboBoostMenuCaption(void)
{
	strcpy(
		comboBoostMenuCaption,
		IsComboBoostEnabled() ? "Combo Boost: On" : "Combo Boost: Off"
	);
}
/*****************************************************************************/
static BOOL IsWalkModeMenuItem(const MENUITEM *mi)
{
	if(!mi || !mi->name)
		return FALSE;
	return (
		(strcmp(mi->name, walkModeMenuCaption) == 0) ||
		(strncmp(mi->name, "Joystick", 8) == 0) ||
		(strncmp(mi->name, "joystick", 8) == 0)
	);
}
/*****************************************************************************/
static BOOL IsAudioModeMenuItem(const MENUITEM *mi)
{
	if(!mi || !mi->name)
		return FALSE;
	return (
		(strcmp(mi->name, audioModeMenuCaption) == 0) ||
		(strncmp(mi->name, "SFX Voice:", 10) == 0) ||
		(strncmp(mi->name, "Audio:", 6) == 0)
	);
}
/*****************************************************************************/
static BOOL IsMusicBackendMenuItem(const MENUITEM *mi)
{
	if(!mi || !mi->name)
		return FALSE;
	return (
		(strcmp(mi->name, musicBackendMenuCaption) == 0) ||
		(strncmp(mi->name, "Music Backend:", 14) == 0) ||
		(strncmp(mi->name, "Music:", 6) == 0)
	);
}
/*****************************************************************************/
static BOOL IsCharacterBoostMenuItem(const MENUITEM *mi)
{
	if(!mi || !mi->name)
		return FALSE;
	return (
		(strcmp(mi->name, characterBoostMenuCaption) == 0) ||
		(strncmp(mi->name, "Move Boost:", 11) == 0)
	);
}
/*****************************************************************************/
static BOOL IsAudioBoostMenuItem(const MENUITEM *mi)
{
	if(!mi || !mi->name)
		return FALSE;
	return (
		(strcmp(mi->name, audioBoostMenuCaption) == 0) ||
		(strncmp(mi->name, "Audio Boost:", 12) == 0)
	);
}
/*****************************************************************************/
static BOOL IsLarrySpeedHackMenuItem(const MENUITEM *mi)
{
	if(!mi || !mi->name)
		return FALSE;
	return (
		(strcmp(mi->name, larrySpeedHackMenuCaption) == 0) ||
		(strncmp(mi->name, "Larry Room Boost:", 17) == 0)
	);
}
/*****************************************************************************/
static BOOL IsComboBoostMenuItem(const MENUITEM *mi)
{
	if(!mi || !mi->name)
		return FALSE;
	return (
		(strcmp(mi->name, comboBoostMenuCaption) == 0) ||
		(strncmp(mi->name, "Combo Boost:", 12) == 0)
	);
}
/*****************************************************************************/
void InitMenuSystem()
{
	menu		= NULL;	// first menu
	menuLast	= NULL;	// last menu
    lastItem	= NULL;	// last item of last menu

    activeMenu	= NULL;	// currently selected
    activeItem	= NULL;

    MENU_SET	= TRUE;

	menuRect.right	= 0;

	mbPtr		= menuBuf;
	UpdateWalkModeMenuCaption();
	UpdateAudioModeMenuCaption();
	UpdateMusicBackendMenuCaption();
	UpdateCharacterBoostMenuCaption();
	UpdateAudioBoostMenuCaption();
	UpdateLarrySpeedHackMenuCaption();
	UpdateComboBoostMenuCaption();
}
/*****************************************************************************/
void FreeMenuSystem()
{            /*
	MENU *m,*m2;
    MENUITEM *mi,*mi2;

    m=menu;
    while(m) {
    	mi=m->items;
    	while(mi) {
         	mi2 = mi->next;
            free(mi);
            mi=mi2;
        }
        m2 = m->next;
        free(m);
        m=m2;
    }                           */
    mbPtr		= menuBuf;  

	menu		= NULL;
	menuLast	= NULL;
    lastItem	= NULL;
    activeMenu	= NULL;
    activeItem	= NULL;

    MENU_SET	= FALSE;
}
/*****************************************************************************/
void SetMenu(char *caption)
{
	MENU *m;
    int col=1;

	if(mbPtr+sizeof(MENU)>=MB_END)
		return;
	m=(MENU*)mbPtr;
	mbPtr+=sizeof(MENU);
	
	
    if(!menu)
    	menu = m;

    m->prev = menuLast;
    m->next = NULL;
    if(menuLast) {
    	menuLast->next = m;
        col = menuLast->column+strlen(menuLast->name)+1;
    }
    menuLast = m;

    m->name		= caption; // no need to allocate, string is const
    m->column	= col;
    m->items	= NULL;
    m->width	= 0;
    m->height	= 0;

    lastItem	= NULL;
}
/*****************************************************************************/
void SetMenuItem(char *caption,U8 ctl)
{
	MENUITEM *mi;
    int row=2,len;

	if(!menuLast)
    	ErrorMessage(ERR_ADDMENUITEM, caption);

	if(mbPtr+sizeof(MENUITEM)>=MB_END)
		return;
	mi=(MENUITEM*)mbPtr;
	mbPtr+=sizeof(MENUITEM);

    if(!menuLast->items)
    	menuLast->items = mi;
    mi->prev = lastItem;
    mi->next = NULL;
    if(lastItem) {
    	lastItem->next = mi;
        row = lastItem->row+1;
    }
	if(caption && ((!strncmp(caption, "Joystick", 8)) || (!strncmp(caption, "joystick", 8)))) {
		UpdateWalkModeMenuCaption();
    	mi->name		= walkModeMenuCaption;
		ctl = 0;
	} else if(caption && ((!strncmp(caption, "SFX Voice:", 10)) || (!strncmp(caption, "Audio:", 6)))) {
		UpdateAudioModeMenuCaption();
		mi->name		= audioModeMenuCaption;
		ctl = 0;
	} else if(caption && ((!strncmp(caption, "Music Backend:", 14)) || (!strncmp(caption, "Music:", 6)))) {
		UpdateMusicBackendMenuCaption();
		mi->name		= musicBackendMenuCaption;
		ctl = 0;
	} else if(caption && ((!strncmp(caption, "Character Boost:", 16)) || (!strncmp(caption, "Move Boost:", 11)))) {
		UpdateCharacterBoostMenuCaption();
		mi->name		= characterBoostMenuCaption;
		ctl = 0;
	} else if(caption && (!strncmp(caption, "Audio Boost:", 12))) {
		UpdateAudioBoostMenuCaption();
		mi->name		= audioBoostMenuCaption;
		ctl = 0;
	} else if(caption && (!strncmp(caption, "Larry Room Boost:", 17))) {
		UpdateLarrySpeedHackMenuCaption();
		mi->name		= larrySpeedHackMenuCaption;
		ctl = 0;
	} else if(caption && (!strncmp(caption, "Combo Boost:", 12))) {
		UpdateComboBoostMenuCaption();
		mi->name		= comboBoostMenuCaption;
		ctl = 0;
	} else
     	mi->name		= caption; // no need to allocate, string is const
	if((len = strlen(mi->name))>menuLast->width)
    	menuLast->width = len;
    mi->row			= row;
    mi->controller	= ctl;
    mi->properties	= MI_ENABLED;

	menuLast->height++;
    lastItem = mi;
}
/*****************************************************************************/
void SubmitMenu()
{
	if(menu) {
    	SetMenu("\x90\0");
        menuLast->column = 39;
    	SetMenuItem("SFX Voice: Original",0);
    	SetMenuItem("Music Backend: Live Tandy",0);
    	SetMenuItem("Move Boost: On",0);
    	SetMenuItem("Audio Boost: On",0);
    	SetMenuItem("Larry Room Boost: On",0);
    	SetMenuItem("Combo Boost: On",0);
    	SetMenuItem("About",0);
    	SetMenuItem("GBAGI Help",0);
    	MENU_SET = TRUE;
    }
}
/*****************************************************************************/
void ToggleMenuItem(U8 ctl,BOOL ENABLE)
{
	MENU *m;
	MENUITEM *mi;

    m=menu;
    while(m) {
    	mi=m->items;
    	while(mi) {
        	if(mi->controller==ctl) {		
				if(ENABLE)
			    	mi->properties |= MI_ENABLED;
			    else
			    	mi->properties &= ~MI_ENABLED;
            }
         	mi = mi->next;
        }
        m = m->next;
    }
}
/*****************************************************************************/
void EnableAllMenuItems()
{
	MENU *m;
	MENUITEM *mi;

    m=menu;
    while(m) {
    	mi=m->items;
    	while(mi) {
        	mi->properties |= MI_ENABLED;
         	mi = mi->next;
        }
        m = m->next;
    }
}
/*****************************************************************************/
void MenuInput()
{
    EVENT *ev;
    BOOL PARSING_MENU;

    if(!TestFlag(fMENU)||!MENU_SELECTABLE)
    	return;

    if(!menu)
    	ErrorMessage(ERR_NOMENUSET);

    if(!activeMenu)
    	activeMenu = menu;

	MENU_ACTIVE = TRUE;
    DrawMenuBar();
	PARSING_MENU = TRUE;
    while(PARSING_MENU) {
    	ev = WaitForEvent();
#ifdef _WINDOWS
    	if(ev == NULL) break; // happens if QUIT_FLAG is true
#endif
        if(ev->type == EV_ASCII) {
        	switch(ev->data) {
        		case KEY_ESC:
                	PARSING_MENU = FALSE;
            		break;
        		case KEY_ENTER:
                    if((!activeItem)||!(activeItem->properties&MI_ENABLED))
                    	break;
					if(IsWalkModeMenuItem(activeItem)) {
						WALK_HOLD = !WALK_HOLD;
						UpdateWalkModeMenuCaption();
						DrawMenuBar();
						PARSING_MENU = FALSE;
						break;
					}
					if(IsAudioModeMenuItem(activeItem)) {
						SetAudioMode(GetNextAudioMode(GetAudioMode()));
						SaveGlobalAudioPreferences();
						UpdateAudioModeMenuCaption();
						DrawMenuBar();
						WriteStatusLine();
						PARSING_MENU = FALSE;
						break;
					}
					if(IsMusicBackendMenuItem(activeItem)) {
						SetAudioMusicBackend(GetNextAudioMusicBackend(GetAudioMusicBackend()));
						SaveGlobalAudioPreferences();
						UpdateMusicBackendMenuCaption();
						DrawMenuBar();
						WriteStatusLine();
						PARSING_MENU = FALSE;
						break;
					}
					if(IsCharacterBoostMenuItem(activeItem)) {
						SetCharacterBoostEnabled(!IsCharacterBoostEnabled());
						SaveGlobalAudioPreferences();
						UpdateCharacterBoostMenuCaption();
						DrawMenuBar();
						WriteStatusLine();
						PARSING_MENU = FALSE;
						break;
					}
					if(IsAudioBoostMenuItem(activeItem)) {
						SetAudioBoostEnabled(!IsAudioBoostEnabled());
						SaveGlobalAudioPreferences();
						UpdateAudioBoostMenuCaption();
						DrawMenuBar();
						WriteStatusLine();
						PARSING_MENU = FALSE;
						break;
					}
					if(IsLarrySpeedHackMenuItem(activeItem)) {
						SetLarryMusicSpeedHackEnabled(!IsLarryMusicSpeedHackEnabled());
						SaveGlobalAudioPreferences();
						UpdateLarrySpeedHackMenuCaption();
						DrawMenuBar();
						WriteStatusLine();
						PARSING_MENU = FALSE;
						break;
					}
					if(IsComboBoostMenuItem(activeItem)) {
						SetComboBoostEnabled(!IsComboBoostEnabled());
						SaveGlobalAudioPreferences();
						UpdateComboBoostMenuCaption();
						DrawMenuBar();
						WriteStatusLine();
						PARSING_MENU = FALSE;
						break;
					}
                 	if(activeMenu==menuLast) {
                     	switch(activeItem->row) {
                         	case 2:
								break;
                         	case 3:
								break;
                         	case 4:
								break;
                         	case 5:
								break;
                         	case 6:
								break;
                         	case 7:
								break;
                         	case 8:
                             	cVersion();
                             	break;
                            case 9:
                             	maxWidth = 38;
								MessageBox(
									"BUTTONS: In Game\n"
									"\n"
									"Up/Down/Left/Right: Movement\n"
									"A:      Brings up text input\n"
									"        dialog (\"ENTER/RETURN\")\n"
									"L:      Repeat previous text\n"
									"        input (\"F3\")\n"
									"B:      Brings up game menu (\"ESC\")\n"
									"Start:  Brings up virtual keyboard\n"
									"R:      Repeat previous key pressed\n"
									"        on virtual keyboard\n"
									"Select: Cycle through three vertical\n"
									"        scrolling positions\n"
									"START+SELECT+A+B:\n"
									"        Exit to Game Select Screen"
								);
                            maxWidth = 38;
								MessageBox(
									"BUTTONS: In GUI\n"
									"\n"
									"L:      Select previous control\n"
									"R:      Select next control\n"
									"A:      Press button\n"
									"Up/Down:\n"
									"        Select item in listbox,\n"
									"        select letter in edit box\n"
									"Left/Right:\n"
									"        Select next/previous set\n" 
									"        of words in list box\n"
									"Start:  Shortcut for pressing OK\n"
									"        in \"Enter Input\" dialog"
								);
                            	break;
                        }
                    } else
                    	controllers[activeItem->controller] = 1;
                	PARSING_MENU = FALSE;
            		break;
            }
            continue;
        }
        if(ev->type!=EV_DIRECTION) continue;
        switch(ev->data) {
        	case dirLEFT:
            	activeMenu = (activeMenu->prev)?
            		activeMenu->prev:menuLast;
                activeItem = NULL;
                DrawMenuBar();
                break;
        	case dirRIGHT:
            	activeMenu = (activeMenu->next)?
            		activeMenu->next:menu;
                activeItem = NULL;
                DrawMenuBar();
                break;
        	case dirUP:
            	if(activeItem) {
                    MENUITEM *newit = activeItem;
                    do {
                    	newit = (newit->prev)?newit->prev:GetLastItem(newit);
                    } while(newit != activeItem && IsMenuSeparator(newit));
                	DrawMenuItems(newit);
                }
                break;
        	case dirDOWN:
            	if(activeItem) {
                    MENUITEM *newit = activeItem;
                    do {
                    	newit = (newit->next)?newit->next:GetFirstItem(newit);
                    } while(newit != activeItem && IsMenuSeparator(newit));
                	DrawMenuItems(newit);
                }
                break;
        }
    }
	MENU_ACTIVE = FALSE;
    ClearMenuItems();
	WriteStatusLine();
}
/*****************************************************************************/
MENUITEM *FindMenuItem(U8 ctl)
{
	MENU *m;
	MENUITEM *mi;

    m=menu;
    while(m) {
    	mi=m->items;
    	while(mi) {
        	if(mi->controller==ctl)
            	return mi;
         	mi = mi->next;
        }
        m = m->next;
    }
    return NULL;
}
/*****************************************************************************/
MENUITEM *GetLastItem(MENUITEM *miStart)
{
	MENUITEM *mi=miStart;
	if(mi)
    	while((BOOL)(mi = mi->next))
    		if(!mi->next)
        		return mi;
    return miStart;
}
/*****************************************************************************/
MENUITEM *GetFirstItem(MENUITEM *miStart)
{
	MENUITEM *mi=miStart;
	if(mi)
    	while((BOOL)(mi = mi->prev))
    		if(!mi->prev)
        		return mi;
    return miStart;
}
/*****************************************************************************/
void DrawMenuBar()
{
	MENU *m;
	PUSH_TEXT_STYLE();

	ClearMenuItems();
#ifdef FANCY_WINDOWS
    ClearLine(0, clWINDOW);
#else
    ClearLine(0, clWHITE);
#endif
	textRow = 0;
    m=menu;
    while(m) {
      	textCol = m->column;
                    
#ifdef FANCY_WINDOWS
        gRect(textCol*(CHAR_WIDTH)-4, 0, ((textCol*(CHAR_WIDTH))+(strlen(m->name)*CHAR_WIDTH))-1-4, CHAR_HEIGHT-1, (m==activeMenu)?clTITLEACTIVE:clWINDOW);
        DrawStringAbs(textCol*CHAR_WIDTH-4,0,m->name,(m==activeMenu)?clWHITE:clBLACK);
#else
        textColour = (m==activeMenu)?0x0F:0xF0;
        DrawString(m->name,FALSE);
#endif
        m = m->next;
    }

    if(activeMenu) {
    	activeItem = activeMenu->items;
    	DrawMenuItems(NULL);
    }      
#ifdef _WINDOWS
    SystemUpdate();
#endif

	POP_TEXT_STYLE();
}
/*****************************************************************************/
void ClearMenuItems()
{
    if(menuRect.right) {
    	RenderUpdate(0,Y_ADJUST_CL-4/*menuRect.top*/,PIC_MAXX,menuRect.bottom+menuRect.top+Y_ADJUST_CL-4,FALSE);
        menuRect.right=0;
    }
}
/*****************************************************************************/
BOOL IsMenuSeparator(MENUITEM *mi)
{
 	if(!mi->properties) {
     	char *s = mi->name;
        while(*s=='-') s++;
        if(!*s) return TRUE;
    }
    return FALSE;
}
/*****************************************************************************/
void DrawMenuItems(MENUITEM *miActive)
{
	MENUITEM *mi;
    int col,x,y;
    int cf,cb;
    col = activeMenu->column;
    if(col+activeMenu->width+2>GBA_MAXCOL)
    	col = GBA_MAXCOL+1-(activeMenu->width+2);
	if(miActive==NULL) {
    	menuRect.left	= (col-1)*(CHAR_WIDTH);
    	menuRect.top	= CHAR_HEIGHT;
    	menuRect.right	= (col+activeMenu->width+2)*(CHAR_WIDTH)-1;
    	menuRect.bottom	= (activeMenu->height+3)*CHAR_HEIGHT;
    	BoxNBorder(menuRect.left,menuRect.top,menuRect.right,menuRect.bottom,0x0F);
        miActive = activeItem;
	}
    mi=activeMenu->items;
    x = col*(CHAR_WIDTH);
    y = 2*CHAR_HEIGHT;
    while(mi) {
    	cb=-1;
#ifdef FANCY_WINDOWS
        if(mi==miActive) {
        	cf = clWHITE;
        	cb = (mi->properties)?clTITLEACTIVE:0xF8;
        } else {
        	cf = (mi->properties)?clBLACK:0xF8;
        	if(mi==activeItem)
            	cb = clWINDOW;
        }
        if(IsMenuSeparator(mi)) {
        	gHLine(menuRect.left+3,menuRect.right-5,y+(CHAR_HEIGHT/2),cl3DSHADOWDK);
        	gHLine(menuRect.left+3,menuRect.right-5,y+(CHAR_HEIGHT/2)+1,cl3DLIGHTDK);
        } else {
        	if(cb!=-1)
        		gRect(x, y, (x+(activeMenu->width*CHAR_WIDTH))-1, y+CHAR_HEIGHT-1, cb);
        	DrawStringAbs(x,y,mi->name,cf);
        }
#else
        if(mi==miActive) {
        	cb = (mi->properties)?clBLACK:clSILVER;
        	cf = clWHITE;
        } else {
        	cf = (mi->properties)?clBLACK:clSILVER;
        	if(mi==activeItem)
            	cb = clWHITE;
        }
        if(cb!=-1)
        	RectFill(x>>1, y, (x+(activeMenu->width*CHAR_WIDTH))>>1, y+CHAR_HEIGHT, cb);
        DrawStringAbs(x,y,mi->name,cf);
#endif
        y+=CHAR_HEIGHT;
        mi = mi->next;
    }
    if(miActive)
    	activeItem = miActive;
#ifdef _WINDOWS
    SystemUpdate();
#endif
}
/*****************************************************************************/

/*****************************************************************************/
