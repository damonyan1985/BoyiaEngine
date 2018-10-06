#include "AppLoader.h"

#define APP_LOAD_URL ""

namespace yanbo
{
LVoid AppLoader::startLoad()
{
	m_loader.loadUrl(APP_LOAD_URL, this);
}

LVoid AppLoader::onDataReceived(const String& data)
{

}

LVoid AppLoader::onStatusCode(LInt statusCode)
{

}

LVoid AppLoader::onFileLen(LInt len)
{

}

LVoid AppLoader::onRedirectUrl(const String& redirectUrl)
{

}

LVoid AppLoader::onLoadError(LInt error)
{

}

LVoid AppLoader::onLoadFinished(const String& data)
{

}
}
