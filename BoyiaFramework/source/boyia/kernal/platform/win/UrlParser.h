#ifndef UrlParser_h
#define UrlParser_h

#include <string>

using std::wstring;

namespace yanbo {
struct Uri
{
	wstring protocol;
	wstring host; 
	wstring path; 
	wstring query;
};
class UrlParser
{
public:
	static void parse(const wstring& url, Uri& uri);
};
}

#endif
