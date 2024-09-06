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
	TLabel *Label1; int Label1X; int Label1Y;
	TLabel *Label2; int Label2X; int Label2Y;
	TLabel *Label3; int Label3X; int Label3Y;
	TPanel *Panel1;
	TPanel *Panel2;
	TPanel *Panel3;
	TPanel *Panel4;
	TListBox *listbox; int listboxX; int listboxY;
	TLabel *Label4;
	TPanel *Panel6;
	TPanel *Panel7;
	TEdit *tbOutput; int tbOutputX; int tbOutputY;
	TEdit *tbVocab; int tbVocabX; int tbVocabY;
	TLabel *Label5;
	TEdit *tbInput; int tbInputX; int tbInputY;
	TLabel *Label6;
	TPanel *Panel8;
	TPanel *Panel9;
	TLabel *Label7; int Label7X; int Label7Y;
	TPanel *Panel10;
	TButton *btnAdd; int btnAddX; int btnAddY;
	TButton *btnRemove; int btnRemoveX; int btnRemoveY;
	TPanel *Panel12;
	TButton *btnBrowseInp; int btnBrowseInpX; int btnBrowseInpY;
	TPanel *Panel13;
	TButton *btnBrowseVoc; int btnBrowseVocX; int btnBrowseVocY;
	TPanel *Panel14;
	TButton *btnBrowseOut; int btnBrowseOutX; int btnBrowseOutY;
	TPanel *FormMain; int FormMainPosX; int FormMainPosY; int FormMainSizeX; int FormMainSizeY;
	TLabel *txStatus; int txStatusPosX; int txStatusPosY; int txStatusSizeX; int txStatusSizeY;
	TPanel *Panel11; int Panel11X; int Panel11Y;
	TButton *btnExit; int btnExitX; int btnExitY;
	TButton *btnBuild; int btnBuildX; int btnBuildY;
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
	BOOL OnSize(UINT width, UINT height);
public:		// User declarations
	__fastcall TFormMain(TComponent* Owner);

    void __fastcall UpdateControls();
    BOOL PackGames();

    TAddGameObj *FindAddGame(int num);
    int RemoveAddGame(int num);
    void RemoveAddGames();

    TDirDialog *DirDialog;

    TAddGameObj *addGameFirst,*addGamePtr;
	TCHAR szPath[4096];

    VclString ProgramDir;

    LPCTSTR GetProgramPath();
};
//---------------------------------------------------------------------------
#endif
