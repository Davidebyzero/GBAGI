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
#include "decompress.h"
#include <conio.h>
/******************************************************************************/   
#define BASE800	0x08000000
#define BASEx0X	0x00020000

#define BASE80X	(BASE800|BASEx0X)

#define IDSIZE	24
char agiid[]="GBAGI 1.0 '''BRIPRO'''\0";

#define IN_FILE "E:\\programming\\gbagi\\gbagi.bin"
#define OUT_FILE "E:\\programming\\gbagi\\gbarom\\gbagi.gba"
/******************************************************************************/
  	// early version 2
GVER v2early =
    {0,
    	0x02, 0x0272};
  	// common version 2
GVER v2common =
    {ENCRYPT_OBJ,
    	0x02, 0x0917};
  	// amiga version 2 equiv
GVER v2amiga =
    {ENCRYPT_OBJ|AMIGA,
    	0x02, 0x0917};
  	// early version 3
GVER v3early =
    {SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ,
    	0x03, 0x2086};
  	// common version 3
GVER v3common =
    {SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ,
    	0x03, 0x2149};
  	// amiga version 3 equiv
GVER v3amiga =
    {SINGLE_DIR|PACKED_DIRS|ENCRYPT_OBJ|AMIGA,
    	0x03, 0x2149};
/******************************************************************************/
GAMEINFO games[] = {
	{&v2common,	"",		"King's Quest 1", 				"D:\\agigames\\King's Quest 1 (AGI)\\"},
	{&v2common,	"",		"King's Quest 2", 				"D:\\agigames\\King's Quest 2\\"},
	{&v3amiga,	"",		"King's Quest 3",				"D:\\agigames\\KingsQuest3\\"},
	{&v3early,	"KQ4",	"King's Quest 4", 				"D:\\agigames\\King's Quest 4 (AGI)\\"},
	{&v3common,	"DM", 	"King's Quest 4 Demo", 			"D:\\agigames\\King's Quest 4 (Demo)\\"},
	{&v2common,	"",		"Leisure Suit Larry",			"D:\\agigames\\Leisure Suit Larry 1\\"},
	{&v3common,	"MH",	"Manhunter 1:New York", 		"D:\\agigames\\Manhunter 1 - New York\\"},
	{&v3common,	"MH2",	"Manhunter 2:San Francisco",	"D:\\agigames\\Manhunter 2 - San Francisco\\"},
	{&v2common, "",		"Police Quest", 				"D:\\agigames\\Police Quest 1 (AGI)\\"},   
	{&v2common, "",		"Space Quest 1", 				"D:\\agigames\\Space Quest 1 (AGI)\\"},
	{&v2common, "",		"Space Quest 2", 				"D:\\agigames\\Space Quest 2\\"},    
	{&v3common,	"GR",	"Gold Rush!",					"D:\\agigames\\Goldrush\\"},
	{&v2common,	"",		"The Black Cauldron", 			"D:\\agigames\\The Black Cauldron\\"},
	{&v2early,	"",   	"Donald Duck's Playground", 	"D:\\agigames\\Donald Duck's Playground\\"},
	{&v2common,	"",   	"Mixed Up Mother Goose", 		"D:\\agigames\\Mixed Up Mother Goose\\"},
	{&v3common,	"DM", 	"Demo Pack #4", 				"D:\\agigames\\demopac4\\"},
};
int totalGames = 16;
/******************************************************************************/
int main()
{
	int i, l;
    FILE *fin;

    if((fin=fopen(IN_FILE,"rb"))==NULL) {
 		ErrorMessage("Error opening input rom!");
		return 1;
    }
    if((fout=fopen(OUT_FILE,"wb"))==NULL) {
    	fclose(fin);
 		ErrorMessage("Error opening file for output!");
		return 1;
    }
    fseek(fin,0,SEEK_END);
    l=ftell(fin);
    fseek(fin,0,SEEK_SET);
    for(i=0;i<l;i++)
    	fputc(fgetc(fin),fout);
    fclose(fin);
    for(i=l;i<BASEx0X;i++)
    	fputc(0xFF,fout);

    offs = BASE80X;

  	fwrite(agiid,IDSIZE,1,fout);
    fputc(totalGames,fout);   
    for(i=IDSIZE+1;i<0x20;i++)
    	fputc(0,fout);
	offs += 0x20;

    giPos = ftell(fout);

    for(i=totalGames*80;i>0;i--)
    	fputc(0,fout);
    offs += totalGames*80;

    clrscr();
    for(i=0;i<totalGames;i++) {
    	gotoxy(1,1);
        printf("Packing game: %s...\t\t\t",games[i].title);
		if(!ProcessGame(&games[i])) {
    		FreeGame();
    		fclose(fout);
            ErrorMessage("Error processing game %s", games[i].title);
            return 1;
        }
    	FreeGame();
    }

	fclose(fout);

	return 0;
}
/******************************************************************************/
