#include "CharConvertor.h"
#include <wchar.h>

namespace yanbo {
std::wstring CharConvertor::CharToWchar(const char* c, size_t encode)
{
    std::wstring str;
    int len = MultiByteToWideChar(encode, 0, c, strlen(c), NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    MultiByteToWideChar(encode, 0, c, strlen(c), wstr, len);
    wstr[len] = L'\0';
    str = wstr;
    delete[] wstr;
    return str;
}

LVoid CharConvertor::WcharToChar(const wchar_t* wp, String& text, size_t encode)
{
    //std::string str;
    int len = WideCharToMultiByte(encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
    char* chs = NEW_BUFFER(char, len + 1);
    WideCharToMultiByte(encode, 0, wp, wcslen(wp), chs, len, NULL, NULL);
    chs[len] = '\0';
    //str = m_char;
    //delete m_char;
    text.Copy(_CS(chs), LTrue, len);
}

LVoid CharConvertor::WcharToChar(const WString& wp, String& text, size_t encode)
{
    int len = WideCharToMultiByte(encode, 0, wp.GetBuffer(), wp.GetLength(), NULL, 0, NULL, NULL);
    char* chs = NEW_BUFFER(char, len + 1);
    WideCharToMultiByte(encode, 0, wp, wcslen(wp), chs, len, NULL, NULL);
    chs[len] = '\0';
    text.Copy(_CS(chs), LTrue, len);
}
}