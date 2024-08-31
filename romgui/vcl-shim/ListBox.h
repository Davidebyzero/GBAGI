#pragma once
#include "vcl-shim.h"

struct TListBox : TComponent {

    struct ListboxList {
        int Count;
        TListBox* self;

        ListboxList(TListBox* self) {
            this->self = self;
            this->Count = 0;
        }

        int Add(const TCHAR* str) {
            int pos = (int)::SendMessage(self->hWnd, LB_ADDSTRING, 0, (LPARAM)str);
            Count++;
            return pos;
        }

        void Delete(int index) {
            ::SendMessage(self->hWnd, LB_DELETESTRING, index, 0);
            Count--;
        }

        void Clear() {
            Count = 0;
            ::SendMessage(self->hWnd, LB_RESETCONTENT, 0, 0);
        }
    };

    ListboxList* Items;
    int ItemIndex;

    TListBox(TComponent* owner) : TComponent(owner) {
        this->Items = new ListboxList(this);
        this->ItemIndex = -1;
    }

    ~TListBox() {
        delete this->Items;
    }

    virtual void Update() {
        TComponent::Update();

        int curSel = ::SendMessage(this->hWnd, LB_GETCURSEL, 0, 0);
        if (curSel != this->ItemIndex) {
            ::SendMessage(this->hWnd, LB_SETCURSEL, this->ItemIndex, 0);
        }
    }

    virtual void NotifyCommand(WPARAM wParam, LPARAM lParam) {
        if (HIWORD(wParam) == LBN_SELCHANGE) {
            this->ItemIndex = ::SendMessage(this->hWnd, LB_GETCURSEL, 0, 0);
        }
    }
};
