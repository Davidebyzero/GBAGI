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
#include "wingui.h"
#include "parse.h"
#include "gamedata.h"
#include "system.h"
#include "screen.h"
#include "text.h"
#include "commands.h"
#include "screen.h"
/*****************************************************************************/
#define MAX_INPUT_LEN	40
char szInput[MAX_INPUT_LEN+1], szInputClean[MAX_INPUT_LEN+1], szString[MAX_INPUT_LEN+1];
/*****************************************************************************/
S16 wnInputProc(WND *w, U16 msg, U16 wParam, U32 lParam);
S16 wnGetStringProc(WND *w, U16 msg, U16 wParam, U32 lParam);

WND wnInput = {
	NULL,NULL,NULL,NULL,
	10,20,220,120,
	{0,0,0,0},{0,0,0,0},
	"Enter Input",
	0,
	0,
	wsRESIZABLE|wsTITLE|wsSELECTABLE,
	(WNPROC)wnInputProc
};
WND bnCancel = {
	NULL,NULL,&wnInput,NULL,
	84,82,60,16,
	{0,0,0,0},{0,0,0,0},
	"Cancel",
	wnBUTTON,
	0,
	bsCAPTION|wsSELECTABLE,
	(WNPROC)wnInputProc
};
WND bnOK = {
	NULL,NULL,&wnInput,NULL,
	150,82,60,16,
	{0,0,0,0},{0,0,0,0},
	"OK",
	wnBUTTON,
	0,
	bsCAPTION|wsSELECTABLE,
	(WNPROC)wnInputProc
};
WND bnMore = {
	NULL,NULL,&wnInput,NULL,
	162,62,48,16,
	{0,0,0,0},{0,0,0,0},
	"",
	wnBUTTON,
	0,
	bsCAPTION|wsSELECTABLE,
	(WNPROC)wnInputProc
};
WND lbWords = {
	NULL,NULL,&wnInput,NULL,
	106,2,102,56,
	{0,0,0,0},{0,0,0,0},
	"Items",
	wnLISTBOX,
	0,
	wsSELECTABLE|wsEXTFIXED,
	(WNPROC)wnInputProc
};
WND lbSelWords = {
	NULL,NULL,&wnInput,NULL,
	2,2,100,56,
	{0,0,0,0},{0,0,0,0},
	"Items",
	wnLISTBOX,
	0,
	wsSELECTABLE|wsEXTFIXED,
	(WNPROC)wnInputProc
};
WND edInput = {
	NULL,NULL,&wnInput,NULL,
	2,62,158,16,
	{0,0,0,0},{0,0,0,0},
	szInput,
	wnEDIT,
	0,
	0,
	(WNPROC)wnInputProc
};
WND wnGetString = {
	NULL,NULL,NULL,NULL,
	10,20,220,120,
	{0,0,0,0},{0,0,0,0},
	"G/S",
	0,
	0,
	wsRESIZABLE|wsSELECTABLE,
	(WNPROC)wnGetStringProc
};
char textField[41];
WND edGSEdit = {
	NULL,NULL,&wnGetString,NULL,
	1,0,0,14,
	{0,0,0,0},{0,0,0,0},
	textField,
	wnEDIT,
	0,
	wsSELECTABLE,
	(WNPROC)wnGetStringProc
};
WND txGSMessage = {
	NULL,NULL,&wnGetString,NULL,
	2,4,0,0,
	{0,0,0,0},{0,0,0,0},
	NULL,
	wnTEXT,
	0,
	txAUTOSIZE,
	(WNPROC)wnGetStringProc
};
WND sbWords,sbWordsSel;
/*****************************************************************************/
U16 input[MAX_INPUT],inpos;
int wordCount;
BOOL MORE_MODE;
char *wordStrings[MAX_INPUT];

#define TOTAL_FAV	7
char *favWords[TOTAL_FAV] = {
 	"look",
    "open",
    "close",
    "talk",
    "use",
    "take",
    "give"
};

char *spaceChars=" ,.?!();:[]{}'`-\"";
/*****************************************************************************/
char *StripInput(char *sStart)
{
	char *s=sStart,*so=szInputClean;
	while(*s) {
        while(*s&&!strchr(spaceChars,*s))
        	*so++ = (*s>='A'&&*s<='Z')?*s++|0x20:*s++;
        while(*s&&strchr(spaceChars,*s)) s++;      
        if(*s&&s!=sStart)
        	*so++ = ' ';
    }
    *so = '\0';
    return szInputClean;
}
/*****************************************************************************/
// thought I was free of doing this since the player input comes from the listboxes,
// wrong I was! I still need to accomodate the "parse(sA);" command!
char *ParseInput(char *sStart)
{
	char *s=StripInput(sStart),*szWord;
    int l,group;
    wordCount = 0;
	while(*s) {
    	l=0;
		if((szWord = FindWordN(s))==NULL) {
         	vars[vUNKWORD] = ++wordCount;
            break;
        }

        group = bGetW(szWord-2)&0x1FFF;
        l = szWord[-3]-4;
        if(group!=9999) {
			wordStrings[wordCount] = s;
        	input[wordCount++] = group;
        }
        if(!s[l]) break;
     	s[l] = '\0';
        s+=l+1;
    }
	if(wordCount)
    	SetFlag(fPLAYERCOMMAND);
   	return wordStrings[0];
}
/*****************************************************************************/
int StrIsInt(char *string)
{
	do
    	if(*string<'0'||*string>'9')
        	return FALSE;
   	while(*++string);
	return TRUE;
}
/*****************************************************************************/
int StrToInt(char *string)
{
	 int num = 0;
     while(*string)
     	num = (num * 10) + (*string++ - '0');
	return num;
}
/*****************************************************************************/
char *FindWord(char *szWord)
{
	char *p=(char*)words[szWord[0]-'a'];
    if(!p) return NULL;
    while(*p) {
     	if(strcmp(szWord,p+3)==0)
        	return p+3;
        p+=*p;
    }
    return NULL;
}
/*****************************************************************************/
// this checks if a word in the tok is contained at the beginning of the specified string
// the strings do not need to be the same length, and the tok work can have spaces!
char *FindWordN(char *szWord)
{
	char *p,*s1,*s2,*possible=NULL;
    if(szWord[0]<'a'||szWord[0]>'z') return NULL;
    if((p = (char*)words[szWord[0]-'a'])=='\0') return NULL;
    while(*p) {
    	s1 = p+3;
        s2 = szWord;
        while(*s1&&*s1++==*s2) s2++;
     	if(!*s1&&(*s2==' '||*s2=='\0')) {
           	if(!possible || strlen(possible)<strlen(p+3)) {
            	possible = p+3; // ensures that "escape pod" will be checked for in the case of another word such as "escape"
            }
        }
        p+=*p;
    }
    return possible;
}
/*****************************************************************************/
void InitParseSystem()
{
	int i;
	char *p;

	memset(input,0,sizeof(input));
	inpos = wordCount = 0;
	memset(wordStrings,0,sizeof(wordStrings));
    szInput[0]='\0';
}
/*****************************************************************************/
void FillListBox(int mode)
{
	U8 *xflg;
    U8 *p;
    int g,a;

	bnMore.caption = (mode)?"‰ Less":"More ˆ";

	WndStopUpdate(&lbWords);
	WndStopUpdate(&lbSelWords);
    ListBoxClear(&lbWords);
    ListBoxClear(&lbSelWords);
    if(vars[vROOMNUM]&&(BOOL)(xflg=logWords[vars[vROOMNUM]])) {
      	for(a=0;a<26;a++) {
			p=words[a];
        	if(!p) continue;
        	while(*p) {
        		g = bGetW(p+1);
        	    if((((xflg[(g&0x1FFF)>>2]>>(((g&0x1FFF)&3)<<1))&1 ))||(mode&&( ((xflg[(g&0x1FFF)>>2]>>(((g&0x1FFF)&3)<<1))&3 )/*==2*/))) {
        	    		if((g&0x8000))
                    		ListBoxAdd(&lbSelWords,(char*)(p+3));
                    	else
        	    			ListBoxAdd(&lbWords,(char*)(p+3));
                }
         	   p+=*p;
        	}
        }
    }
	ListBoxSetScrollbar(&lbWords,&sbWords);
	ListBoxSelect(&lbWords,0);
	ListBoxSetScrollbar(&lbSelWords,&sbWordsSel);
	ListBoxSelect(&lbSelWords,0);
	WndStartUpdate(&lbWords);
	WndStartUpdate(&lbSelWords);
    wDrawWnd(&bnMore,1);

    MORE_MODE = mode;
}
/*****************************************************************************/
void ExecuteInputDialog(BOOL CLEAR)
{

    if(CLEAR) {
 		inpos=0;
    	szInput[0] = '\0';
    } else {
     	inpos = wordCount;
    }

    AddWindow(&wnInput);
    AddWindow(&bnOK);
    AddWindow(&bnCancel);
    AddWindow(&bnMore);
    AddWindow(&edInput);
    AddWindow(&lbWords);
    AddWindow(&lbSelWords);
	ListBoxSelect(&lbSelWords,0);

	FillListBox(0);

	WinGUIDoit();
}
/*****************************************************************************/
S16 wnInputProc(WND *w, U16 msg, U16 wParam, U32 lParam)
{
	U8 *p;
	switch(msg) {
    	case wmLISTBOX_CLICK:
        	//if(w==&lbWords) {
             	if(wParam==KEY_ENTER) {
                	if(inpos<MAX_INPUT) {
             			p = (U8*)lParam;
             			input[inpos++] = bGetW(p-2);    
                        wordStrings[inpos]=szInput+strlen(szInput);
             			strcat(szInput,(char*)p);
             			strcat(szInput," ");
             			wDrawWnd(&edInput,TRUE);
                    }
             	} else if(wParam==KEY_ESC) {
                    if(inpos) {
                        wordStrings[inpos][0]='\0';
             			inpos--;
             			wDrawWnd(&edInput,TRUE);
                    }
             	}
            //}
            break;
    	case wmBUTTON_CLICK:
        	if(wParam==KEY_START||w==&bnOK) {
             	WndDispose(&wnInput);
				ParseInput(szInput);
            } else if(wParam==KEY_SELECT||w==&bnCancel) {
             	WndDispose(&wnInput);
            } else if(w==&bnMore) {
             	FillListBox(!MORE_MODE);
            }
            break;
    }
	return TRUE;
}
/*****************************************************************************/
BOOL GET_INT, dest;
void ExecuteGetStringDialog(BOOL _GET_INT, U8 _dest, char *msg, int maxLen)
{
    char *szMsg;

    GET_INT	= _GET_INT;
    dest	= _dest;
	if(GET_INT) {
     	edGSEdit.style |= esDIGITONLY;
    } else {
     	if(dest >= MAX_STRINGS)
        	return;
     	if(maxLen >= MAX_STRINGS_LEN)
        	maxLen = MAX_STRINGS_LEN;
     	edGSEdit.style &= ~esDIGITONLY;
    }
    AddWindow(&wnGetString);
    AddWindow(&edGSEdit);
    AddWindow(&txGSMessage);

	wnGetString.caption = GET_INT?"Enter a Number":"Enter a String";
    maxWidth = 38;
    szMsg = WordWrap(msg);

    edGSEdit.ext.edit.maxLen = maxLen;
    edGSEdit.ext.edit.col=0;
    szInput[0]='\0';

    if(maxLen>maxWidth)
    	maxLen = maxWidth;
    if(msgWidth<maxLen)
    	msgWidth = maxLen;

    wnGetString.width	= (3+msgWidth)*(CHAR_WIDTH);
    wnGetString.height	= (msgHeight)*CHAR_HEIGHT+32;
    wnGetString.x		= (SCREEN_WIDTH-wnGetString.width)>>1;
    wnGetString.y		= (SCREEN_HEIGHT-wnGetString.height);
    CalcWndRect(&wnGetString);
    txGSMessage.caption	= szMsg;
    CalcWndRect(&txGSMessage);
	edGSEdit.width		= (maxLen+1)*CHAR_WIDTH;
	edGSEdit.y			= txGSMessage.rect.bottom+4;
    szString[0]			= '\0';
    edGSEdit.caption	= szString;
    CalcWndRect(&edGSEdit);

	WinGUIDoit();

    maxWidth = -1;
}
/*****************************************************************************/
S16 wnGetStringProc(WND *w, U16 msg, U16 wParam, U32 lParam)
{
	int val;
	switch(msg) {
    	case wmBUTTON_CLICK:
        	if(wParam==KEY_START||wParam==KEY_A) {
                if(GET_INT) {
                   	val = StrIsInt(szString)?StrToInt(szString):0;
                    vars[dest] = (val>255)?0:val;
                } else { // string
                	strcpy(strings[dest],szString);
                }
             	WndDispose(&wnGetString);
            }
            break;
    }
	return TRUE;
}
/*****************************************************************************/

