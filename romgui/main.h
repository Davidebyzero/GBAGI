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

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include "vcl-shim/vcl-shim.h"
 
#include "dirdialog.h"
#include "makerom.h"
#include "decompress.h"
#include "commands.h"
#include "verdef.h"
//---------------------------------------------------------------------------
class TAddGameObj
{
public:
	TAddGameObj *prev,*next;
    GAMEINFO gameinfo;
    int itemindex;
};
//---------------------------------------------------------------------------
class TFormMain : public TForm
{
__published:	// IDE-managed Components
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TPanel *Panel1;
	TPanel *Panel2;
	TPanel *Panel3;
	TPanel *Panel4;
	TListBox *listbox;
	TLabel *Label4;
	TPanel *Panel6;
	TPanel *Panel7;
	TEdit *tbOutput;
	TEdit *tbVocab;
	TLabel *Label5;
	TEdit *tbInput;
	TLabel *Label6;
	TPanel *Panel8;
	TPanel *Panel9;
	TLabel *Label7;
	TPanel *Panel10;
	TButton *btnAdd;
	TButton *btnRemove;
	TPanel *Panel12;
	TButton *btnBrowseInp;
	TPanel *Panel13;
	TButton *btnBrowseVoc;
	TPanel *Panel14;
	TButton *btnBrowseOut;
	TPanel *FormMain;
	TLabel *txStatus;
	TPanel *Panel11;
	TButton *btnExit;
	TButton *btnBuild;
	TOpenDialog *dlgOpenInp;
	TOpenDialog *dlgOpenVoc;
	TSaveDialog *dlgSaveOut;
	void __fastcall FormResize(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall btnAddClick(TObject *Sender);
	void __fastcall btnBrowseInpClick(TObject *Sender);
	void __fastcall btnBrowseVocClick(TObject *Sender);
	void __fastcall btnBrowseOutClick(TObject *Sender);
	void __fastcall btnRemoveClick(TObject *Sender);
	void __fastcall btnBuildClick(TObject *Sender);
	void __fastcall btnExitClick(TObject *Sender);
	void __fastcall tbOutputChange(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall Label3Click(TObject *Sender);
private:	// User declarations
	void CreateControls();
	void OnInitDialog(HWND hWnd);
	void OnCommand(WPARAM wParam, LPARAM lParam);
public:		// User declarations
	__fastcall TFormMain(TComponent* Owner);

    void __fastcall UpdateControls();
    BOOL PackGames();

    TAddGameObj *FindAddGame(int num);
    int RemoveAddGame(int num);
    void RemoveAddGames();

    TDirDialog *DirDialog;

    TAddGameObj *addGameFirst,*addGamePtr;
	char szPath[4096];

    AnsiString ProgramDir;

    LPCTSTR GetProgramPath();
};
//---------------------------------------------------------------------------
#endif
