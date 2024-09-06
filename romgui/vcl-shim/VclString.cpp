#include "VclString.h"
#include <windows.h>

#ifdef UNICODE
char *strdup_tchar_to_char(const TCHAR *ws)
{
    size_t length = _tcslen(ws);
    char *s = (char*)malloc(length + 1);
    WideCharToMultiByte(CP_OEMCP, 0, ws, -1, s, length, nullptr, nullptr);
    s[length] = '\0';
    return s;
}

VclString::VclString(const char* s)
{
    size_t length = strlen(s);
    wchar_t *ws = new wchar_t [length + 1];
    MultiByteToWideChar(CP_OEMCP, 0, s, -1, ws, length);
    ws[length] = L'\0';
    *this = ws;
}
#endif
