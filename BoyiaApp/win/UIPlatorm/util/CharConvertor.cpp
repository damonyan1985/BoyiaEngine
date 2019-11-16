#include "CharConvertor.h"
#include <wchar.h>

namespace yanbo {
std::wstring CharConvertor::CharToWchar(const char* c, size_t encode)
{
	std::wstring str;
	int len = MultiByteToWideChar(encode, 0, c, strlen(c), NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	MultiByteToWideChar(encode, 0, c, strlen(c), wstr, len);
	wstr[len] = '\0';
	str = wstr;
	delete wstr;
	return str;
}

std::string CharConvertor::WcharToChar(const wchar_t* wp, size_t encode)
{
	std::string str;
	int len = WideCharToMultiByte(encode, 0, wp, wcslen(wp), NULL, 0, NULL, NULL);
	char* m_char = new char[len + 1];
	WideCharToMultiByte(encode, 0, wp, wcslen(wp), m_char, len, NULL, NULL);
	m_char[len] = '\0';
	str = m_char;
	delete m_char;
	return str;
}
}