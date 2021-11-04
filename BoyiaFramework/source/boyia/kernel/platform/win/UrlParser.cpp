#include "UrlParser.h"
#include <string>
#include <algorithm>
#include <cctype>
#include <functional>
#include <iterator>

namespace yanbo
{
void UrlParser::parse(const wstring& url, Uri& uri)
{
	const wstring prot_end(L"://");
	wstring::const_iterator prot_i = search(url.begin(), url.end(),
		prot_end.begin(), prot_end.end());
	uri.protocol.reserve(distance(url.begin(), prot_i));
	transform(url.begin(), prot_i,
		back_inserter(uri.protocol),
		tolower); // protocol is icase
	if (prot_i == url.end())
		return;
	advance(prot_i, prot_end.length());
	wstring::const_iterator path_i = find(prot_i, url.end(), '/');
	uri.host.reserve(distance(prot_i, path_i));
	transform(prot_i, path_i,
		back_inserter(uri.host),
		tolower); // host is icase
	wstring::const_iterator query_i = find(path_i, url.end(), '?');
	uri.path.assign(path_i, query_i);
	if (query_i != url.end())
		++query_i;
	uri.query.assign(query_i, url.end());
}
}