#include "LoaderWin.h"

#if ENABLE(BOYIA_WINDOWS)

namespace yanbo {
LoaderWin::LoaderWin()
{
}
LoaderWin::~LoaderWin()
{
}

LVoid LoaderWin::syncLoadUrl(const String& url, String& content)
{
}
LVoid LoaderWin::loadUrl(const String& url, NetworkClient* client)
{
}
LVoid LoaderWin::loadUrl(const String& url, NetworkClient* client, LBool isWait)
{
}

LVoid LoaderWin::postData(const String& url, NetworkClient* client)
{
}
LVoid LoaderWin::postData(const String& url, NetworkClient* client, LBool isWait)
{
}
LVoid LoaderWin::cancel()
{
}

NetworkBase* NetworkBase::create()
{
	return new LoaderWin();
}
}

#endif