#pragma once

// Based on https://hamgit.ir/parikhaleghi/libunistd/-/blob/master/vcl/vcl.h

#include <string>
#include <tchar.h>

#ifdef UNICODE
#   define __stdstring std::wstring
#else
#   define __stdstring std::string
#endif

#ifdef UNICODE
char *strdup_tchar_to_char(const TCHAR *ws);
#else
#   define strdup_tchar_to_char _strdup
#endif

class VclString
    : public __stdstring
{
public:
    int Length() const
    {
        return length();
    }
    VclString()
    {}
    VclString(const TCHAR* s)
        : __stdstring(s)
    {}
#ifdef UNICODE
    VclString(const char* s);
#endif
    VclString(const TCHAR* s, int offset, int len)
        : __stdstring(s, offset, len)
    {}
    VclString(const __stdstring& s)
        : __stdstring(s)
    {}
    VclString SubString(int offset, int len) const
    {
        return VclString(c_str(), offset, len);
    }
    int Pos(const TCHAR* s) const
    {
        const TCHAR* p = c_str();
        const TCHAR* offset = _tcsstr(c_str(), s);
        if (!offset)
        {
            return 0;
        }
        return offset - p;
    }
    int ToInt() const
    {
        return _tstoi(c_str());
    }
    VclString& operator=(const TCHAR* s)
    {
        assign(s);
        return *this;
    }
    VclString& operator=(const __stdstring& s)
    {
        assign(s);
        return *this;
    }
    VclString operator+(const TCHAR* s)
    {
        VclString n(*this);
        n.append(s);
        return n;
    }
    VclString operator+(const VclString& s)
    {
        VclString n(*this);
        n.append(s);
        return n;
    }
    operator const TCHAR* () const
    {
        return c_str();
    }
    void printf(const TCHAR* f, int x)
    {}
};
