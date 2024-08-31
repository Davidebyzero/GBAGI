#pragma once

#include <string>

// Based on https://hamgit.ir/parikhaleghi/libunistd/-/blob/master/vcl/vcl.h

class AnsiString
    : public std::string
{
public:
    int Length() const
    {
        return length();
    }
    AnsiString()
    {}
    AnsiString(const char* s)
        : std::string(s)
    {}
    AnsiString(const char* s, int offset, int len)
        : std::string(s, offset, len)
    {}
    AnsiString(const std::string& s)
        : std::string(s)
    {}
    AnsiString SubString(int offset, int len) const
    {
        return AnsiString(c_str(), offset, len);
    }
    int Pos(const char* s) const
    {
        const char* p = c_str();
        const char* offset = strstr(c_str(), s);
        if (!offset)
        {
            return 0;
        }
        return offset - p;
    }
    int ToInt() const
    {
        return atoi(c_str());
    }
    AnsiString& operator=(const char* s)
    {
        assign(s);
        return *this;
    }
    AnsiString& operator=(const std::string& s)
    {
        assign(s);
        return *this;
    }
    AnsiString operator+(const char* s)
    {
        AnsiString n(*this);
        n.append(s);
        return n;
    }
    AnsiString operator+(const AnsiString& s)
    {
        AnsiString n(*this);
        n.append(s);
        return n;
    }
    operator const char* () const
    {
        return c_str();
    }
    void printf(const char* f, int x)
    {}
};
