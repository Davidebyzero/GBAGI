#include "vocabedit.res.h"
#include "vocabedit.h"

void TFormVocabEdit::OnInitDialog(HWND hWnd) {
    Attach(hWnd);
    lblGame = new TLabel(this); lblGame->Attach(GetDlgItem(hWnd, IDC_LABELGAME));
    lblHelp = new TLabel(this); lblHelp->Attach(GetDlgItem(hWnd, IDC_LABELHELP));
    lblStatus = new TLabel(this); lblStatus->Attach(GetDlgItem(hWnd, IDC_LABELSTATUS));
    GroupGroups = new TGroupBox(this); GroupGroups->Attach(GetDlgItem(hWnd, IDC_GROUPGROUPS));
    GroupWords = new TGroupBox(this); GroupWords->Attach(GetDlgItem(hWnd, IDC_GROUPWORDS));
    listGroups = new TListBox(this); listGroups->Attach(GetDlgItem(hWnd, IDC_LISTGROUPS));
    listWords = new TListBox(this); listWords->Attach(GetDlgItem(hWnd, IDC_LISTWORDS));
    btnKeep = new TButton(this); btnKeep->Attach(GetDlgItem(hWnd, IDC_BTNKEEP));
    btnRemoveWord = new TButton(this); btnRemoveWord->Attach(GetDlgItem(hWnd, IDC_BTNREMOVEWORD));
    btnHide = new TButton(this); btnHide->Attach(GetDlgItem(hWnd, IDC_BTNHIDE));
    btnClearWord = new TButton(this); btnClearWord->Attach(GetDlgItem(hWnd, IDC_BTNCLEARWORD));
    btnClearGroup = new TButton(this); btnClearGroup->Attach(GetDlgItem(hWnd, IDC_BTNCLEARGROUP));
    btnColLeft = new TButton(this); btnColLeft->Attach(GetDlgItem(hWnd, IDC_BTNCOLLEFT));
    btnColRight = new TButton(this); btnColRight->Attach(GetDlgItem(hWnd, IDC_BTNCOLRIGHT));
    btnColAuto = new TButton(this); btnColAuto->Attach(GetDlgItem(hWnd, IDC_BTNCOLAUTO));
    btnShowAuto = new TButton(this); btnShowAuto->Attach(GetDlgItem(hWnd, IDC_BTNSHOWAUTO));
    btnShowNormal = new TButton(this); btnShowNormal->Attach(GetDlgItem(hWnd, IDC_BTNSHOWNORMAL));
    btnShowMore = new TButton(this); btnShowMore->Attach(GetDlgItem(hWnd, IDC_BTNSHOWMORE));
    tbSearch = new TEdit(this); tbSearch->Attach(GetDlgItem(hWnd, IDC_EDITSEARCH));
    tbAlias = new TEdit(this); tbAlias->Attach(GetDlgItem(hWnd, IDC_EDITALIAS));
    btnAddAlias = new TButton(this); btnAddAlias->Attach(GetDlgItem(hWnd, IDC_BTNADDALIAS));
    btnLoad = new TButton(this); btnLoad->Attach(GetDlgItem(hWnd, IDC_BTNLOADVOCAB));
    btnSave = new TButton(this); btnSave->Attach(GetDlgItem(hWnd, IDC_BTNSAVEVOCAB));
    btnResetAuto = new TButton(this); btnResetAuto->Attach(GetDlgItem(hWnd, IDC_BTNRESETAUTO));
    GroupPreview = new TGroupBox(this); GroupPreview->Attach(GetDlgItem(hWnd, IDC_GROUPPREVIEW));
    listPreviewLeft = new TListBox(this); listPreviewLeft->Attach(GetDlgItem(hWnd, IDC_LISTPREVIEWLEFT));
    listPreviewRight = new TListBox(this); listPreviewRight->Attach(GetDlgItem(hWnd, IDC_LISTPREVIEWRIGHT));
    btnPreviewToggle = new TButton(this); btnPreviewToggle->Attach(GetDlgItem(hWnd, IDC_BTNPREVIEWTOGGLE));
    btnOK = new TButton(this); btnOK->Attach(GetDlgItem(hWnd, IDC_BTNOKVOCAB));
    btnCancel = new TButton(this); btnCancel->Attach(GetDlgItem(hWnd, IDC_BTNCANCELVOCAB));

    {
        HWND ownerDrawButtons[] = {
            btnColLeft->hWnd, btnColRight->hWnd,
            btnShowNormal->hWnd, btnShowMore->hWnd
        };
        for(int i = 0; i < 4; ++i) {
            LONG_PTR style = ::GetWindowLongPtr(ownerDrawButtons[i], GWL_STYLE);
            style &= ~BS_TYPEMASK;
            style |= BS_OWNERDRAW;
            ::SetWindowLongPtr(ownerDrawButtons[i], GWL_STYLE, style);
            ::InvalidateRect(ownerDrawButtons[i], NULL, TRUE);
        }
    }

    if(btnColAuto && btnColAuto->hWnd)
        ::ShowWindow(btnColAuto->hWnd, SW_HIDE);
    if(btnShowAuto && btnShowAuto->hWnd)
        ::ShowWindow(btnShowAuto->hWnd, SW_HIDE);

    lblGame->SetFont(-16, FW_SEMIBOLD, FALSE, FALSE, FALSE, "Segoe UI");
    lblHelp->SetFont(-13, FW_NORMAL, FALSE, FALSE, FALSE, "Segoe UI");
    GroupGroups->SetFont(-14, FW_SEMIBOLD, FALSE, FALSE, FALSE, "Segoe UI");
    GroupWords->SetFont(-14, FW_SEMIBOLD, FALSE, FALSE, FALSE, "Segoe UI");
    GroupPreview->SetFont(-14, FW_SEMIBOLD, FALSE, FALSE, FALSE, "Segoe UI");
    btnOK->SetFont(-14, FW_SEMIBOLD, FALSE, FALSE, FALSE, "Segoe UI");
}

void TFormVocabEdit::OnCommand(WPARAM wParam, LPARAM lParam) {
    TForm::OnCommand(wParam, lParam);
    if (LOWORD(wParam) == IDC_LISTGROUPS && HIWORD(wParam) == LBN_SELCHANGE) listGroupsChange(this);
    if (LOWORD(wParam) == IDC_LISTWORDS && HIWORD(wParam) == LBN_SELCHANGE) listWordsChange(this);
    if (LOWORD(wParam) == IDC_EDITSEARCH && HIWORD(wParam) == EN_CHANGE) tbSearchChange(this);
    if (LOWORD(wParam) == IDC_BTNKEEP && HIWORD(wParam) == BN_CLICKED) btnKeepClick(this);
    if (LOWORD(wParam) == IDC_BTNREMOVEWORD && HIWORD(wParam) == BN_CLICKED) btnRemoveWordClick(this);
    if (LOWORD(wParam) == IDC_BTNHIDE && HIWORD(wParam) == BN_CLICKED) btnHideClick(this);
    if (LOWORD(wParam) == IDC_BTNCLEARWORD && HIWORD(wParam) == BN_CLICKED) btnClearWordClick(this);
    if (LOWORD(wParam) == IDC_BTNCLEARGROUP && HIWORD(wParam) == BN_CLICKED) btnClearGroupClick(this);
    if (LOWORD(wParam) == IDC_BTNCOLLEFT && HIWORD(wParam) == BN_CLICKED) btnColLeftClick(this);
    if (LOWORD(wParam) == IDC_BTNCOLRIGHT && HIWORD(wParam) == BN_CLICKED) btnColRightClick(this);
    if (LOWORD(wParam) == IDC_BTNCOLAUTO && HIWORD(wParam) == BN_CLICKED) btnColAutoClick(this);
    if (LOWORD(wParam) == IDC_BTNSHOWAUTO && HIWORD(wParam) == BN_CLICKED) btnShowAutoClick(this);
    if (LOWORD(wParam) == IDC_BTNSHOWNORMAL && HIWORD(wParam) == BN_CLICKED) btnShowNormalClick(this);
    if (LOWORD(wParam) == IDC_BTNSHOWMORE && HIWORD(wParam) == BN_CLICKED) btnShowMoreClick(this);
    if (LOWORD(wParam) == IDC_BTNADDALIAS && HIWORD(wParam) == BN_CLICKED) btnAddAliasClick(this);
    if (LOWORD(wParam) == IDC_BTNLOADVOCAB && HIWORD(wParam) == BN_CLICKED) btnLoadClick(this);
    if (LOWORD(wParam) == IDC_BTNSAVEVOCAB && HIWORD(wParam) == BN_CLICKED) btnSaveClick(this);
    if (LOWORD(wParam) == IDC_BTNRESETAUTO && HIWORD(wParam) == BN_CLICKED) btnResetAutoClick(this);
    if (LOWORD(wParam) == IDC_BTNPREVIEWTOGGLE && HIWORD(wParam) == BN_CLICKED) btnPreviewToggleClick(this);
    if (LOWORD(wParam) == IDC_BTNOKVOCAB && HIWORD(wParam) == BN_CLICKED) btnOKClick(this);
    if (LOWORD(wParam) == IDC_BTNCANCELVOCAB && HIWORD(wParam) == BN_CLICKED) btnCancelClick(this);
}

LRESULT TFormVocabEdit::OnDrawItem(WPARAM wParam, LPARAM lParam) {
    DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT*)lParam;
    if(!dis)
        return FALSE;

    switch(dis->CtlID) {
        case IDC_LISTWORDS:
            break;
        case IDC_BTNCOLLEFT:
        case IDC_BTNCOLRIGHT:
        case IDC_BTNSHOWNORMAL:
        case IDC_BTNSHOWMORE:
            break;
        default:
            return FALSE;
    }

    TCHAR text[128];
    RECT rc = dis->rcItem;
    bool active = false;
    HBRUSH hBrush;
    HPEN hPen;
    COLORREF textColor;

    if(dis->CtlID == IDC_LISTWORDS) {
        TCHAR itemText[512];
        RECT fill = dis->rcItem;
        COLORREF bg = (dis->itemState & ODS_SELECTED) ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_WINDOW);
        COLORREF fg = ::GetSysColor(COLOR_WINDOWTEXT);
        int groupIndex = GetSelectedGroupVectorIndex();
        bool used = false;

        itemText[0] = 0;
        if(dis->itemID != (UINT)-1)
            ::SendMessage(dis->hwndItem, LB_GETTEXT, dis->itemID, (LPARAM)itemText);

        if(groupIndex >= 0 && groupIndex < (int)groups.size() &&
           dis->itemID != (UINT)-1 && dis->itemID < visibleWords.size()) {
            used = IsEffectivelyUsed(groups[groupIndex].group, visibleWords[dis->itemID].c_str());
        }

        if(!(dis->itemState & ODS_SELECTED))
            fg = used ? RGB(0, 102, 204) : ::GetSysColor(COLOR_WINDOWTEXT);
        else
            fg = ::GetSysColor(COLOR_HIGHLIGHTTEXT);

        hBrush = ::CreateSolidBrush(bg);
        ::FillRect(dis->hDC, &fill, hBrush);
        ::DeleteObject(hBrush);
        ::SetTextColor(dis->hDC, fg);
        ::SetBkMode(dis->hDC, TRANSPARENT);
        fill.left += 3;
        ::DrawText(dis->hDC, itemText, -1, &fill, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        if(dis->itemState & ODS_FOCUS) {
            RECT focus = dis->rcItem;
            ::InflateRect(&focus, -1, -1);
            ::DrawFocusRect(dis->hDC, &focus);
        }
        return TRUE;
    }

    ::GetWindowText(dis->hwndItem, text, 128);
    active = (_tcsncmp(text, _T(">> "), 3) == 0);

    hBrush = ::CreateSolidBrush(active ? RGB(0, 120, 215) : ::GetSysColor(COLOR_BTNFACE));
    ::FillRect(dis->hDC, &rc, hBrush);
    ::DeleteObject(hBrush);

    hPen = ::CreatePen(PS_SOLID, 1, active ? RGB(0, 84, 153) : ::GetSysColor(COLOR_BTNSHADOW));
    HPEN oldPen = (HPEN)::SelectObject(dis->hDC, hPen);
    HGDIOBJ oldBrush = ::SelectObject(dis->hDC, ::GetStockObject(HOLLOW_BRUSH));
    ::Rectangle(dis->hDC, rc.left, rc.top, rc.right, rc.bottom);
    ::SelectObject(dis->hDC, oldBrush);
    ::SelectObject(dis->hDC, oldPen);
    ::DeleteObject(hPen);

    textColor = active ? RGB(255,255,255) : ::GetSysColor(COLOR_BTNTEXT);
    ::SetTextColor(dis->hDC, textColor);
    ::SetBkMode(dis->hDC, TRANSPARENT);
    ::DrawText(dis->hDC, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    if(dis->itemState & ODS_FOCUS) {
        RECT focus = rc;
        ::InflateRect(&focus, -3, -3);
        ::DrawFocusRect(dis->hDC, &focus);
    }

    return TRUE;
}

void TFormVocabEdit::CreateControls() {
    this->hWnd = CreateDialogParam(NULL, MAKEINTRESOURCE(IDC_TFORMVOCABEDIT), this->Owner->hWnd, TFormDialogProc, (LPARAM)this);
}
