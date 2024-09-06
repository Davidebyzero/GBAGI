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

#include "vcl-shim/vcl-shim.h"
#include <shlobj.h>
#pragma hdrstop

#include "dirdialog.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

TDirDialog *ddPtr;
//---------------------------------------------------------------------------
int __stdcall BrowseProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData )
{
    TCHAR szDir[MAX_PATH];

    switch(uMsg)
    {
        case BFFM_INITIALIZED:
            // Set the initial directory. If WPARAM is TRUE, then LPARAM is a
            // string that contains the path. If WPARAM is FALSE, then LPARAM
            // should be a lovely PIDL
            if(ddPtr->InitialDir!=_T("")) SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)ddPtr->InitialDir.c_str());
            SetWindowText(hwnd,ddPtr->Title.c_str());
            ddPtr->Handle = hwnd;
            
            ddPtr->CheckMap(ddPtr->InitialDir.c_str());
            break;
        case BFFM_SELCHANGED:
            if(SHGetPathFromIDList((LPITEMIDLIST)lParam, szDir))
                ddPtr->CheckMap(szDir);
            break;
    }
    return 0;
}
//---------------------------------------------------------------------------
TDirDialog::TDirDialog()
{
    VclString Title = _T("Select a folder.");
    InitialDir = _T("");
    MAP_CHECK = FALSE;
}       
//---------------------------------------------------------------------------
TCHAR DirectoryString[2048];
void TDirDialog::CheckMap(const TCHAR *szDir)
{
    BOOL FILE_FOUND;
    WIN32_FIND_DATA FindFileData;

	if(!MAP_CHECK) return;

    if(szDir[3])
        wsprintf(DirectoryString, _T("%s\\words.tok"), szDir);
    else
        wsprintf(DirectoryString, _T("%swords.tok"), szDir);

    FILE_FOUND = FindFirstFile((LPCTSTR)DirectoryString,(LPWIN32_FIND_DATA)&FindFileData)!=INVALID_HANDLE_VALUE;
 	if(!FILE_FOUND) {
    	SendMessage(Handle, BFFM_SETSTATUSTEXT, FALSE, (LPARAM)_T("AGI Game exists: NO"));
   	} else {
    	SendMessage(Handle, BFFM_SETSTATUSTEXT, FALSE, (LPARAM)_T("AGI Game exists: YES"));
    }
    SendMessage(Handle, BFFM_ENABLEOK, 0, LPARAM(FILE_FOUND));
}
//---------------------------------------------------------------------------
BOOL TDirDialog::Execute()
{
    BROWSEINFO    info;
    TCHAR         szDir[MAX_PATH];
    TCHAR         szDisplayName[MAX_PATH];
    LPITEMIDLIST  pidl;
    LPMALLOC      pShellMalloc;

    ddPtr = this;
    FullPath = InitialDir;

    if(SHGetMalloc(&pShellMalloc) == NO_ERROR)
    {
        memset(&info, 0x00, sizeof(info));
        info.hwndOwner = 0;
        info.pidlRoot  = NULL;
        info.pszDisplayName = szDisplayName;
        info.lpszTitle = Caption.c_str();
        info.ulFlags   = BIF_RETURNONLYFSDIRS;
        if(MAP_CHECK) info.ulFlags|=BIF_STATUSTEXT;
        info.lpfn      = BrowseProc;             // callback function

        pidl = SHBrowseForFolder(&info);

        if(pidl)
        {
            if(SHGetPathFromIDList(pidl, szDir))
            {
            	int sl=_tcsclen(szDir);
    			if(szDir[sl-1]=='\\')
             		szDir[sl-1]='\0';
                FullPath = VclString(szDir);
            }

            pShellMalloc->Free(pidl);
        }
		pShellMalloc->Release();
    }
    return pidl!=NULL;
}                               
//---------------------------------------------------------------------------