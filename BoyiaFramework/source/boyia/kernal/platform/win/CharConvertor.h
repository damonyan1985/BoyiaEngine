#ifndef CharConvertor_h
#define CharConvertor_h

#include <string>
#include<windows.h>

using std::string;
using std::wstring;

namespace yanbo {
class CharConvertor {
public:
	static wstring CharToWchar(const char* c, size_t m_encode = CP_ACP);
	string WcharToChar(const wchar_t* wp, size_t m_encode = CP_ACP);
};
}

#endif