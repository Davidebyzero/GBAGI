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

//---------------------------------------------------------------------------

#include <vcl-shim.h>
#pragma hdrstop

#include "addgame.h"
#include "main.h"

#if defined(_MSC_VER) 
#define strdup _strdup
#endif

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"

//---------------------------------------------------------------------------
__fastcall TFormAddGame::TFormAddGame(TComponent* Owner)
	: TForm(Owner)
{
	CreateControls();

	okClose = FALSE;
}
//---------------------------------------------------------------------------
void TFormAddGame::UpdateControls()
{
	tbId->Enabled = (dropVersion->ItemIndex >= 3)? true : false;
    tbId->Color = tbId->Enabled? clWindow: clBtnFace;

	// Generated
	this->Update();
	dropGames->Update();
	dropVersion->Update();
	tbId->Update();
	tbName->Update();
	tbPath->Update();
}
//---------------------------------------------------------------------------
void TFormAddGame::SetUp(AnsiString path)
{
	tbPath->Text = path;

	dropGames->Items->Clear();
	dropVersion->Items->Clear();

	VERLIST *vl = verlist;
	while(vl->name) {
		dropVersion->Items->Add((vl->name));
		vl++;
	}

	GAMEINFO *gi = games;
	while(gi->title) {
		dropGames->Items->Add((gi->title));
		gi++;
	}

	dropGames->ItemIndex	= dropGames->Items->Count-1; // "<Other>"
	dropVersion->ItemIndex	= 1; // version 2 common
	tbId->Enabled = false;

	char *s = GetPathFolderName();
	tbName->Text = s;
	delete s;

    UpdateControls();
}   
//---------------------------------------------------------------------------
char *TFormAddGame::GetPathString()
{
	char *pathstr = new char[tbPath->Text.Length()+tbId->Text.Length()+32];
	int pathlen = tbPath->Text.Length(), i;

    strcpy(pathstr, tbPath->Text.c_str());

	if(pathlen && pathstr[pathlen-1] == '\\')
		pathlen--;
	pathstr[pathlen] = '\0';

	return pathstr;
}
//---------------------------------------------------------------------------
char *TFormAddGame::GetPathFolderName()
{
	char *pathstr;
	int pathlen,i;

	pathstr = GetPathString();
	pathlen = strlen(pathstr);

	for(i=pathlen-1; i >= 0; i--) {
		if(pathstr[i] == '\\') {
			strcpy(pathstr,pathstr+i+1);
			break;
		}
	}
	if(strlen(pathstr)>25)
		pathstr[25]='\0';
	return pathstr;
}
//---------------------------------------------------------------------------
void __fastcall TFormAddGame::btnCancelClick(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFormAddGame::btnOKClick(TObject *Sender)
{
	gameinfo.version	= &verlist[dropVersion->ItemIndex];
	gameinfo.vID		= strdup((dropVersion->ItemIndex >= 3 )? tbId->Text.c_str():"") ;
	gameinfo.title		= strdup(tbName->Text.c_str());
	gameinfo.path		= strdup((tbPath->Text+"\\").c_str());

	okClose = TRUE;
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TFormAddGame::dropVersionChange(TObject *Sender)
{
	UpdateControls();
}
//---------------------------------------------------------------------------
void __fastcall TFormAddGame::btnAutodetectClick(TObject *Sender)
{
	char *pathstr;
	long len;

	pathstr = GetPathString();
	strcat(pathstr,"\\agidata.ovl");

	if(!FileExists(pathstr))
		ShowMessage("Could not find file: "+AnsiString(pathstr));
	else {
		VERLIST *version = FindAGIVersion(pathstr);
		if(version) {
			char s[128];
			sprintf(s,"Found Version: %s, %d.%04X",version->name,version->ver.major,version->ver.minor);
			ShowMessage(
					s,
					"Version found!"
				);
			dropVersion->ItemIndex = version - verlist;
		} else {
			ShowMessage(
					"Unable to find version",
					"Version not found!"
				);
		}
	}
	delete pathstr;    
	UpdateControls();
}
//---------------------------------------------------------------------------
void __fastcall TFormAddGame::dropGamesChange(TObject *Sender)
{
	if(dropGames->ItemIndex != -1) {
		if(dropGames->Text != "<Other>") {
			GAMEINFO *gi 			= games+dropGames->ItemIndex;
			tbName->Text			= gi->title;
			tbId->Text				= gi->vID;
			dropVersion->ItemIndex = gi->version - verlist; 
			UpdateControls();
		}
	}
}
//---------------------------------------------------------------------------
