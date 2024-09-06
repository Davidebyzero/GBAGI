#pragma once

#include <windows.h>
#include <vector>
#include <algorithm>
#include "VclString.h"

#define __published public
#define __fastcall
#define clWindow GetSysColor(COLOR_WINDOW)
#define clWindowText GetSysColor(COLOR_WINDOWTEXT)
#define clBtnFace GetSysColor(COLOR_BTNFACE)
#define clBlue RGB(0, 0, 255)
#define clWhite RGB(255, 255, 255)

typedef std::exception Exception;

VclString GetCurrentDir();
BOOL FileExists(LPCTSTR szPath);
void ShowMessage(const VclString& Message, const VclString& Title = _T("romgui"));

struct TObject {
    virtual ~TObject() {}
};

struct TComponent : TObject {
    TComponent* Owner;
    HWND hWnd;
    int Left;
    int Right;
    int Width;
    int Height;
    int ClientWidth;
    int ClientHeight;
    BOOL Enabled;
    HCURSOR hCursor;
    HFONT hFont;
    DWORD Color;
    HBRUSH hBgColor;
    std::vector<TComponent*> Controls;

    TComponent(TComponent* owner) {
        this->Owner = owner;
        this->Left = 0;
        this->Right = 0;
        this->Width = 0;
        this->Height = 0;
        this->ClientWidth = 0;
        this->ClientHeight = 0;
        this->hWnd = NULL;
        this->Enabled = TRUE;
        this->hCursor = NULL;
        this->hFont = NULL;
        this->Color = 0;
        this->hBgColor = NULL;
        
        if (this->Owner) {
            this->Owner->Controls.push_back(this);
        }
    }

    ~TComponent() {
        if (this->Owner) {
            auto at = std::find(this->Owner->Controls.begin(), this->Owner->Controls.end(), this); // != vec.end()
            if (at != this->Owner->Controls.end()) {
                this->Owner->Controls.erase(at);
            }
        }

        // Controls remove themselves so delete from a copy
        auto controls = this->Controls;
        for (auto i=controls.begin(); i!=controls.end(); ++i) {
            delete *i;
        }

        if (hCursor) {
            ::DeleteObject(hCursor);
            hCursor = NULL;
        }

        if (hFont) {
            ::DeleteObject(hFont);
            hFont = NULL;
        }

        if (this->hBgColor) {
            ::DeleteObject(this->hBgColor);
            this->hBgColor = NULL;
        }
    }

    void SetFocus() {
        ::SetFocus(this->hWnd);
    }

    void Repaint() {
        ::RedrawWindow(this->hWnd, NULL, NULL, 0);
    }

    virtual void Update() {
        if (::IsWindowEnabled(this->hWnd) != Enabled) {
            ::EnableWindow(this->hWnd, Enabled);
        }
    }

    virtual void Attach(HWND hWnd) {
        this->hWnd = hWnd;
        ::SetWindowLongPtr(this->hWnd, GWLP_USERDATA, (LONG_PTR)this);
    }

    VclString GetText() {
        TCHAR currentText[1024];
        ::GetWindowText(this->hWnd, currentText, 1024);
        return currentText;
    }

    void SetFont(int cHeight, int cWeight, BOOL bItalic, DWORD bUnderline, DWORD bStrikeOut, LPCSTR pszFaceName) {
        if (this->hFont) {
            ::DeleteObject(this->hFont);
        }

        this->hFont = ::CreateFont(cHeight, 0, 0, 0, cWeight, bItalic, bUnderline, bStrikeOut, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("Verdana"));
        ::PostMessage(this->hWnd, WM_SETFONT, (WPARAM)this->hFont, TRUE);
    }

    void SetBgColor(DWORD color) {
        if (this->hBgColor) {
            ::DeleteObject(this->hBgColor);
        }

        this->hBgColor = ::CreateSolidBrush(color);
    }

    virtual void NotifyCommand(WPARAM wParam, LPARAM lParam) {
        // Override in respective controls to update internal state
    }

    virtual LRESULT NotifyCtlColorStatic(WPARAM wParam, LPARAM lParam) {
        // Override in respective static controls, panel, edit
        return FALSE;
    }

    virtual LRESULT NotifyCtlColorEdit(WPARAM wParam, LPARAM lParam) {
        // Override in respective edit controls
        return FALSE;
    }
};

#include "StdCtrls.h"
#include "Form.h"
#include "ComboBox.h"
#include "ListBox.h"
