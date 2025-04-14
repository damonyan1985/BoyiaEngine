#ifndef CharConvertor_h
#define CharConvertor_h

#include "UtilString.h"

#include <string>
#include <windows.h>

//using std::string;
using std::wstring;

using WString = util::LString<wchar_t>;

namespace yanbo {
class CharConvertor {
public:
    static wstring CharToWchar(const char* c, size_t encode = CP_UTF8);
    static LVoid WcharToChar(const wchar_t* wp, String& text, size_t encode = CP_UTF8);
    static LVoid WcharToChar(const WString& wp, String& text, size_t encode = CP_UTF8);
};
}

#endif