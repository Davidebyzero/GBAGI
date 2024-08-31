#include "addgame.res.h"
#include "addgame.h"

// Hand-written reimagination of what the auto-generated VCL code by Borland C++ Builder might looked like

void TFormAddGame::OnInitDialog(HWND hWnd) {
    Attach(hWnd);

    this->Label1 = new TLabel(this);
    this->Label1->Attach(GetDlgItem(hWnd, IDC_LABEL1));

    this->Label2 = new TLabel(this);
    this->Label2->Attach(GetDlgItem(hWnd, IDC_LABEL2));

    this->Label3 = new TLabel(this);
    this->Label3->Attach(GetDlgItem(hWnd, IDC_LABEL3));

    this->Label4 = new TLabel(this);
    this->Label4->Attach(GetDlgItem(hWnd, IDC_LABEL4));

    this->Label5 = new TLabel(this);
    this->Label5->Attach(GetDlgItem(hWnd, IDC_LABEL5));

    this->Panel11 = new TPanel(this);
    this->Panel11->Attach(GetDlgItem(hWnd, IDC_PANEL11));

    this->Panel5 = new TPanel(this);
    this->Panel5->Attach(GetDlgItem(hWnd, IDC_PANEL5));

    this->btnAutodetect = new TButton(this);
    this->btnAutodetect->Attach(GetDlgItem(hWnd, IDC_BTNAUTODETECT));

    this->btnCancel = new TButton(this);
    this->btnCancel->Attach(GetDlgItem(hWnd, IDC_BTNCANCEL));

    this->btnOK = new TButton(this);
    this->btnOK->Attach(GetDlgItem(hWnd, IDC_BTNOK));

    this->dropGames = new TComboBox(this);
    this->dropGames->Attach(GetDlgItem(hWnd, IDC_DROPGAMES));

    this->dropVersion = new TComboBox(this);
    this->dropVersion->Attach(GetDlgItem(hWnd, IDC_DROPVERSION));

    this->tbId = new TEdit(this);
    this->tbId->Attach(GetDlgItem(hWnd, IDC_TBID));

    this->tbName = new TEdit(this);
    this->tbName->Attach(GetDlgItem(hWnd, IDC_TBNAME));

    this->tbPath = new TEdit(this);
    this->tbPath->Attach(GetDlgItem(hWnd, IDC_TBPATH));

    this->GroupBox1 = new TGroupBox(this);
    this->GroupBox1->Attach(GetDlgItem(hWnd, IDC_GROUPBOX1));

    this->GroupBox2 = new TGroupBox(this);
    this->GroupBox2->Attach(GetDlgItem(hWnd, IDC_GROUPBOX2));
}

void TFormAddGame::OnCommand(WPARAM wParam, LPARAM lParam) {
    // Update state of internal controls first
    TForm::OnCommand(wParam, lParam);

    if (LOWORD(wParam) == IDC_BTNAUTODETECT && HIWORD(wParam) == BN_CLICKED) {
        this->btnAutodetectClick(this);
    }

    if (LOWORD(wParam) == IDC_BTNCANCEL && HIWORD(wParam) == BN_CLICKED) {
        this->btnCancelClick(this);
    }

    if (LOWORD(wParam) == IDC_BTNOK && HIWORD(wParam) == BN_CLICKED) {
        this->btnOKClick(this);
    }

    if (LOWORD(wParam) == IDC_DROPGAMES && HIWORD(wParam) == CBN_SELCHANGE) {
        this->dropGamesChange(this);
    }

    if (LOWORD(wParam) == IDC_DROPVERSION && HIWORD(wParam) == CBN_SELCHANGE) {
        this->dropGamesChange(this);
    }
}

void TFormAddGame::CreateControls() {
    this->hWnd = CreateDialogParam(NULL, MAKEINTRESOURCE(IDC_TFORMADDGAME), this->Owner->hWnd, TFormDialogProc, (LPARAM)this);
}
