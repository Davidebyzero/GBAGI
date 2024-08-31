#include "Form.h"

INT_PTR CALLBACK TFormDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    TForm* form;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            form = (TForm*)lParam;
            form->OnInitDialog(hWnd);
            return TRUE;
        case WM_COMMAND:
            form = (TForm*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
            form->OnCommand(wParam, lParam);
            break;
        case WM_CTLCOLORSTATIC:
            form = (TForm*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
            return form->OnCtlColorStatic(wParam, lParam);
        case WM_CLOSE:
            form = (TForm*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (form->Owner == NULL) {
                ::DestroyWindow(hWnd);
            } else {
                ::PostMessage(hWnd, WM_ENDDIALOG, 0, 0);
            }
            break;
        case WM_ENDDIALOG:
            // Error if getting here, should be trapped by dialog message loop
            break;
        case WM_SETCURSOR:
            form = (TForm*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
            return form->OnSetCursor(wParam, lParam);
        case WM_DESTROY:
            form = (TForm*)::GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (form->Owner == NULL) {
                ::PostQuitMessage(0);
            }
            return 0;

    }
    return FALSE;
}
