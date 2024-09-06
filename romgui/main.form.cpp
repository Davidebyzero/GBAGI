#include "main.res.h"
#include "main.h"

// Hand-written reimagination of what the auto-generated VCL code by Borland C++ Builder might looked like

void TFormMain::OnInitDialog(HWND hWnd) {
    Attach(hWnd);

    PostMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(GetModuleHandle(NULL), IDI_APPLICATION));
    PostMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(NULL), IDI_APPLICATION));

    this->Label1 = new TLabel(this);
    this->Label1->Attach(GetDlgItem(hWnd, IDC_LABEL1)); // GBAGI Injection Utility
    this->Label1->SetFont(-16, FW_BOLD, FALSE, FALSE, FALSE, "Verdana");

    this->Label2 = new TLabel(this);
    this->Label2->Attach(GetDlgItem(hWnd, IDC_LABEL2)); // By Brian Provinciano
    this->Label2->SetFont(-13, FW_NORMAL, FALSE, FALSE, FALSE, "Verdana");

    this->Label3 = new TLabel(this);
    this->Label3->Attach(GetDlgItem(hWnd, IDC_LABEL3)); // http://www.bripro.com
    this->Label3->SetFont(-11, FW_NORMAL, FALSE, TRUE, FALSE, "Verdana");
    this->Label3->Color = clBlue;
    this->Label3->hCursor = LoadCursor(NULL, IDC_HAND);

    this->Label4 = new TLabel(this);
    this->Label4->Attach(GetDlgItem(hWnd, IDC_LABEL4));

    this->Label5 = new TLabel(this);
    this->Label5->Attach(GetDlgItem(hWnd, IDC_LABEL5));

    this->Label6 = new TLabel(this);
    this->Label6->Attach(GetDlgItem(hWnd, IDC_LABEL6)); // ROM Input Filename

    this->Label7 = new TLabel(this);
    this->Label7->Attach(GetDlgItem(hWnd, IDC_LABEL7)); // Games To Inject
    this->Label7->SetFont(-16, FW_BOLD, FALSE, FALSE, FALSE, "Verdana");
    this->Label7->SetBgColor(11829830);
    this->Label7->Color = clWhite;

    this->txStatus = new TLabel(this);
    this->txStatus->Attach(GetDlgItem(hWnd, IDC_TXSTATUS));

    this->Panel1 = new TPanel(this);
    this->Panel2 = new TPanel(this);
    this->Panel3 = new TPanel(this);
    this->Panel4 = new TPanel(this);
    this->Panel6 = new TPanel(this);
    this->Panel7 = new TPanel(this);
    this->Panel8 = new TPanel(this);
    this->Panel9 = new TPanel(this);
    this->Panel10 = new TPanel(this);
    this->Panel11 = new TPanel(this);
    this->Panel12 = new TPanel(this);
    this->Panel13 = new TPanel(this);
    this->Panel14 = new TPanel(this);
    this->FormMain = new TPanel(this);
    this->Panel1->Attach(GetDlgItem(hWnd, IDC_PANEL1));
    this->Panel2->Attach(GetDlgItem(hWnd, IDC_PANEL2));
    this->Panel3->Attach(GetDlgItem(hWnd, IDC_PANEL3));
    this->Panel4->Attach(GetDlgItem(hWnd, IDC_PANEL4));
    this->Panel6->Attach(GetDlgItem(hWnd, IDC_PANEL6));
    this->Panel7->Attach(GetDlgItem(hWnd, IDC_PANEL7));
    this->Panel8->Attach(GetDlgItem(hWnd, IDC_PANEL8));
    this->Panel9->Attach(GetDlgItem(hWnd, IDC_PANEL9));
    this->Panel10->Attach(GetDlgItem(hWnd, IDC_PANEL10));
    this->Panel11->Attach(GetDlgItem(hWnd, IDC_PANEL11));
    this->Panel12->Attach(GetDlgItem(hWnd, IDC_PANEL12));
    this->Panel13->Attach(GetDlgItem(hWnd, IDC_PANEL13));
    this->Panel14->Attach(GetDlgItem(hWnd, IDC_PANEL14));
    this->FormMain->Attach(GetDlgItem(hWnd, IDC_FORMMAIN));

    this->btnAdd = new TButton(this);
    this->btnAdd->Attach(GetDlgItem(hWnd, IDC_BTNADD));

    this->btnBrowseInp = new TButton(this);
    this->btnBrowseInp->Attach(GetDlgItem(hWnd, IDC_BTNBROWSEINP));

    this->btnBrowseOut = new TButton(this);
    this->btnBrowseOut->Attach(GetDlgItem(hWnd, IDC_BTNBROWSEOUT));

    this->btnBrowseVoc = new TButton(this);
    this->btnBrowseVoc->Attach(GetDlgItem(hWnd, IDC_BTNBROWSEVOC));

    this->btnBuild = new TButton(this);
    this->btnBuild->Attach(GetDlgItem(hWnd, IDC_BTNBUILD));

    this->btnRemove = new TButton(this);
    this->btnRemove->Attach(GetDlgItem(hWnd, IDC_BTNREMOVE));

    this->btnExit = new TButton(this);
    this->btnExit->Attach(GetDlgItem(hWnd, IDC_BTNEXIT));

    this->tbOutput = new TEdit(this);
    this->tbOutput->Attach(GetDlgItem(hWnd, IDC_TBOUTPUT));

    this->tbInput = new TEdit(this);
    this->tbInput->Attach(GetDlgItem(hWnd, IDC_TBINPUT));

    this->tbVocab = new TEdit(this);
    this->tbVocab->Attach(GetDlgItem(hWnd, IDC_TBVOCAB));

    this->listbox = new TListBox(this);
    this->listbox->Attach(GetDlgItem(hWnd, IDC_LISTBOX));

    OPENFILENAME ofnSaveOut;
    ZeroMemory(&ofnSaveOut, sizeof(OPENFILENAME)); // dlgSaveOut
    ofnSaveOut.hwndOwner = hWnd;
    ofnSaveOut.lStructSize = sizeof(OPENFILENAME);
    ofnSaveOut.Flags = 0x0000 | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
    ofnSaveOut.lpstrFile = new TCHAR[260];
    ofnSaveOut.nMaxFile = 260;
    _tcscpy(ofnSaveOut.lpstrFile, _T(""));
    ofnSaveOut.lpstrFilter = _T("GBA ROM (*.gba)\0*.gba\0");
    ofnSaveOut.nFilterIndex = 0;
    ofnSaveOut.lpstrTitle = _T("");
    ofnSaveOut.lpstrDefExt = _T("gba");
    ofnSaveOut.lpstrInitialDir = _T("");

    this->dlgSaveOut = new TSaveDialog(this, ofnSaveOut);

    OPENFILENAME ofnOpenVoc;
    ZeroMemory(&ofnOpenVoc, sizeof(OPENFILENAME)); // dlgOpenVoc
    ofnOpenVoc.hwndOwner = hWnd;
    ofnOpenVoc.lStructSize = sizeof(OPENFILENAME);
    ofnOpenVoc.Flags = 0x0000 | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
    ofnOpenVoc.lpstrFile = new TCHAR[260];
    ofnOpenVoc.nMaxFile = 260;
    _tcscpy(ofnOpenVoc.lpstrFile, _T(""));
    ofnOpenVoc.lpstrFilter = _T("VOCAB.BIN\0vocab.bin\0");
    ofnOpenVoc.nFilterIndex = 0;
    ofnOpenVoc.lpstrTitle = _T("");
    ofnOpenVoc.lpstrDefExt = _T("");
    ofnOpenVoc.lpstrInitialDir = _T("");
    this->dlgOpenVoc = new TOpenDialog(this, ofnOpenVoc);

    OPENFILENAME ofnOpenInp;
    ZeroMemory(&ofnOpenInp, sizeof(OPENFILENAME)); // dlgOpenInp
    ofnOpenInp.hwndOwner = hWnd;
    ofnOpenInp.lStructSize = sizeof(OPENFILENAME);
    ofnOpenInp.Flags = 0x0000 | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ENABLESIZING;
    ofnOpenInp.lpstrFile = new TCHAR[260];
    ofnOpenInp.nMaxFile = 260;
    _tcscpy(ofnOpenInp.lpstrFile, _T(""));
    ofnOpenInp.lpstrFilter = _T("GBAGI.BIN\0gbagi.bin\0");
    ofnOpenInp.nFilterIndex = 0;
    ofnOpenInp.lpstrTitle = _T("");
    ofnOpenInp.lpstrDefExt = _T("");
    ofnOpenInp.lpstrInitialDir = _T("");
    this->dlgOpenInp = new TOpenDialog(this, ofnOpenInp);
}

void TFormMain::OnCommand(WPARAM wParam, LPARAM lParam) {
    // Update state of internal controls first
    TForm::OnCommand(wParam, lParam);

    if (LOWORD(wParam) == IDC_BTNADD && HIWORD(wParam) == BN_CLICKED) {
        this->btnAddClick(this); // sender=lParam->HWND->TControl? unused anyway
    }

    if (LOWORD(wParam) == IDC_BTNREMOVE && HIWORD(wParam) == BN_CLICKED) {
        this->btnRemoveClick(this);
    }

    if (LOWORD(wParam) == IDC_BTNEXIT && HIWORD(wParam) == BN_CLICKED) {
        this->btnExitClick(this);
    }

    if (LOWORD(wParam) == IDC_BTNBROWSEINP && HIWORD(wParam) == BN_CLICKED) {
        this->btnBrowseInpClick(this);
        this->UpdateControls();
    }

    if (LOWORD(wParam) == IDC_BTNBROWSEOUT && HIWORD(wParam) == BN_CLICKED) {
        this->btnBrowseOutClick(this);
        this->UpdateControls();
    }

    if (LOWORD(wParam) == IDC_BTNBROWSEVOC && HIWORD(wParam) == BN_CLICKED) {
        this->btnBrowseVocClick(this);
        this->UpdateControls();
    }

    if (LOWORD(wParam) == IDC_BTNBUILD && HIWORD(wParam) == BN_CLICKED) {
        this->btnBuildClick(this);
        this->UpdateControls();
    }

    if (LOWORD(wParam) == IDC_TBOUTPUT && HIWORD(wParam) == EN_CHANGE) {
        this->tbOutputChange(this);
        this->UpdateControls();
    }

    if (LOWORD(wParam) == IDC_LABEL3 && HIWORD(wParam) == STN_CLICKED) {
        this->Label3Click(this);
    }
}

void TFormMain::CreateControls() {
    this->hWnd = CreateDialogParam(NULL, MAKEINTRESOURCE(IDC_TFORMMAIN), NULL, TFormDialogProc, (LPARAM)this);
}
