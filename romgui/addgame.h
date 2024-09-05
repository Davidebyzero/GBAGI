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

#ifndef addgameH
#define addgameH
//---------------------------------------------------------------------------
#include "vcl-shim/vcl-shim.h"

#include "makerom.h"
#include "decompress.h"
#include "commands.h"   
#include "verdef.h"
//---------------------------------------------------------------------------
class TFormAddGame : public TForm
{
__published:	// IDE-managed Components
	TGroupBox *GroupBox1;
	TGroupBox *GroupBox2;
	TPanel *Panel5;
	TPanel *Panel11;
	TButton *btnOK;
	TButton *btnCancel;
	TComboBox *dropGames;
	TLabel *Label1;
	TEdit *tbPath;
	TComboBox *dropVersion;
	TLabel *Label2;
	TEdit *tbName;
	TLabel *Label3;
	TEdit *tbId;
	TLabel *Label4;
	TButton *btnAutodetect;
	TLabel *Label5;
	void __fastcall btnCancelClick(TObject *Sender);
	void __fastcall btnOKClick(TObject *Sender);
	void __fastcall dropVersionChange(TObject *Sender);
	void __fastcall btnAutodetectClick(TObject *Sender);
	void __fastcall dropGamesChange(TObject *Sender);
private:	// User declarations
	virtual void CreateControls();
	virtual void OnInitDialog(HWND hWnd);
	virtual void OnCommand(WPARAM wParam, LPARAM lParam);
public:		// User declarations
	__fastcall TFormAddGame(TComponent* Owner);

    void SetUp(AnsiString path);
    
	char *GetPathString(void);
	char *GetPathFolderName(void);

    void UpdateControls();


    BOOL okClose;
    GAMEINFO gameinfo;
};
//---------------------------------------------------------------------------
#endif
