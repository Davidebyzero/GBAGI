#pragma once
#include "vcl-shim.h"

#define WM_ENDDIALOG WM_USER + 1

INT_PTR CALLBACK TFormDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct TForm : TComponent {

    TForm(TComponent* owner) : TComponent(owner) {
    }
  
    void ShowModal() {
        HWND hWndParent = ::GetParent(this->hWnd);
        ::EnableWindow(hWndParent, FALSE);

        ::ShowWindow(this->hWnd, SW_SHOW);
        MSG msg = { };
        while (::IsWindow(this->hWnd) && ::GetMessage(&msg, NULL, 0, 0) > 0)
        {
            if (msg.hwnd == this->hWnd && msg.message == WM_ENDDIALOG) {
                ::EnableWindow(hWndParent, TRUE);
                ::DestroyWindow(hWnd);
                continue;
            }

            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }

    void CreateForm() {
        ::ShowWindow(this->hWnd, SW_SHOW);
    }

    void Close() {
        ::PostMessage(this->hWnd, WM_CLOSE, 0, 0);
    }

    virtual void CreateControls() = 0;
    virtual void OnInitDialog(HWND hWnd) = 0;

    virtual void OnCommand(WPARAM wParam, LPARAM lParam) {
        if (!lParam) {
            return;
        }

        HWND hControlWnd = (HWND)lParam;
        TComponent* control = (TComponent*)::GetWindowLongPtr(hControlWnd, GWLP_USERDATA);
        if (!control) {
            return;
        }

        control->NotifyCommand(wParam, lParam);
    }

    virtual LRESULT OnCtlColorStatic(WPARAM wParam, LPARAM lParam) {
        HWND hControlWnd = (HWND)lParam;
        TComponent* control = (TComponent*)::GetWindowLongPtr(hControlWnd, GWLP_USERDATA);
        if (!control) {
            return 0;
        }

        return control->NotifyCtlColorStatic(wParam, lParam);
    }

    virtual LRESULT OnCtlColorEdit(WPARAM wParam, LPARAM lParam) {
        HWND hControlWnd = (HWND)lParam;
        TComponent* control = (TComponent*)GetWindowLongPtr(hControlWnd, GWLP_USERDATA);
        if (!control) {
            return 0;
        }

        return control->NotifyCtlColorEdit(wParam, lParam);
    }

    virtual BOOL OnSetCursor(WPARAM wParam, LPARAM lParam) {
        HWND hControlWnd = (HWND)wParam;
        TComponent* control = (TComponent*)::GetWindowLongPtr(hControlWnd, GWLP_USERDATA);
        if (!control || !control->hCursor) {
            return FALSE;
        }

        ::SetCursor(control->hCursor);
        // Tell child component to not override the cursor
        ::SetWindowLongPtr(this->hWnd, DWLP_MSGRESULT, TRUE);
        return TRUE;
    }

    virtual BOOL OnSize(UINT width, UINT height) {
        return TRUE;
    }
};
