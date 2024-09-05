#pragma once
#include "vcl-shim.h"

struct TButton : TComponent {
    TButton(TComponent* owner) : TComponent(owner) {
    }
};

struct TGroupBox : TComponent {
    TGroupBox(TComponent* owner) : TComponent(owner) {
    }
};

struct TEdit : TComponent {
    AnsiString Text;

    TEdit(TComponent* owner) : TComponent(owner) {
        this->Color = clWindow;
        this->hBgColor = ::GetSysColorBrush(COLOR_WINDOWTEXT);
    }

    virtual void Update() {
        TComponent::Update();

        if (this->GetText() != Text) {
            ::SetWindowText(this->hWnd, Text.c_str());
        }
    }

    virtual void NotifyCommand(WPARAM wParam, LPARAM lParam) {
        if (HIWORD(wParam) == EN_CHANGE) {
            this->Text = this->GetText();
        }
    }

    virtual LRESULT NotifyCtlColorStatic(WPARAM wParam, LPARAM lParam) {
        // Called on EDIT controls when readonly
        return FALSE;
    }

    virtual LRESULT NotifyCtlColorEdit(WPARAM wParam, LPARAM lParam) {
        HDC hdcStatic = (HDC)wParam;
        ::SetTextColor(hdcStatic, this->Color);
        return (LRESULT)this->hBgColor;
    }
};

struct TLabel : TComponent {
    AnsiString Caption;

    TLabel(TComponent* owner) : TComponent(owner) {
        this->Color = clWindowText;
        this->hBgColor = (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
    }

    virtual void Update() {
        TComponent::Update();

        TCHAR currentText[1024];
        ::GetWindowText(this->hWnd, currentText, 1024);
        if (currentText != this->Caption) {
            ::SetWindowText(this->hWnd, this->Caption.c_str());
        }
    }

    virtual LRESULT NotifyCtlColorStatic(WPARAM wParam, LPARAM lParam) {
        HDC hdcStatic = (HDC)wParam;
        ::SetTextColor(hdcStatic, this->Color);
        ::SetBkMode(hdcStatic, TRANSPARENT);
        return (LRESULT)this->hBgColor;
    }
};

struct TPanel : TLabel {
    // Both TPanel and TLabel are Win32 STATIC controls, so just alias TPanel to TLabel
    // Label has transparent background, and Panel has solid
    TPanel(TComponent* owner) : TLabel(owner) {
        this->Color = clWindowText;
        this->hBgColor = ::GetSysColorBrush(COLOR_WINDOW);
    }
};

struct TOpenDialog : TComponent {
    AnsiString FileName;
    OPENFILENAME dlgOfn;

    TOpenDialog(TComponent* owner, const OPENFILENAME& ofn) : TComponent(owner) {
        this->dlgOfn = ofn;
    }

    ~TOpenDialog() {
        delete this->dlgOfn.lpstrFile;
    }

    BOOL Execute() {
        BOOL bRetValdlgSaveOut = ::GetOpenFileName(&dlgOfn);
        FileName = dlgOfn.lpstrFile;
        return bRetValdlgSaveOut;
    }
};

struct TSaveDialog : TComponent {
    AnsiString FileName;
    OPENFILENAME dlgOfn;

    TSaveDialog(TComponent* owner, const OPENFILENAME& ofn) : TComponent(owner) {
        this->dlgOfn = ofn;
    }

    ~TSaveDialog() {
        delete this->dlgOfn.lpstrFile;
    }

    BOOL Execute() {
        BOOL bRetValdlgSaveOut = ::GetSaveFileName(&dlgOfn);
        FileName = dlgOfn.lpstrFile;
        return bRetValdlgSaveOut;
    }
};
