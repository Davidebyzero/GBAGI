#pragma once
#include "vcl-shim.h"

struct TComboBox : TComponent {
    struct ComboboxList {
        int Count;
        TComboBox* self;

        ComboboxList(TComboBox* self) {
            this->self = self;
            this->Count = 0;
        }

        int Add(const TCHAR* str) {
            int pos = (int)::SendMessage(self->hWnd, CB_ADDSTRING, 0, (LPARAM)str);
            Count++;
            return pos;
        }

        void Delete(int index) {
            ::SendMessage(self->hWnd, CB_DELETESTRING, 0, (LPARAM)index);
            Count--;
        }

        void Clear() {
            Count = 0;
            ::SendMessage(self->hWnd, CB_RESETCONTENT, 0, 0);
        }
    };

    ComboboxList* Items;
    int ItemIndex;
    AnsiString Text;

    TComboBox(TComponent* owner) : TComponent(owner) {
        this->Items = new ComboboxList(this);
        this->ItemIndex = -1;
    }

    ~TComboBox() {
        delete this->Items;
    }

    virtual void Update() {
        TComponent::Update();

        int curSel = ::SendMessage(this->hWnd, CB_GETCURSEL, 0, 0);
        if (curSel != this->ItemIndex) {
            ::SendMessage(this->hWnd, CB_SETCURSEL, this->ItemIndex, 0);
        }
    }

    virtual void NotifyCommand(WPARAM wParam, LPARAM lParam) {
        if (HIWORD(wParam) == CBN_SELCHANGE) {
            this->ItemIndex = ::SendMessage(this->hWnd, CB_GETCURSEL, 0, 0);
        }
    }
};
