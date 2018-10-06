/**
 * AppLoader
 * Use For Boyia Apps Download and Load
 * Date: 2018-10-6
 * Author: yanbo
 * All Copyright Reserved
 */
#ifndef AppLoader_h
#define AppLoader_h

#include "Loader.h"

namespace yanbo
{
class AppLoader : public NetworkClient
{
public:
	LVoid startLoad();

    virtual LVoid onDataReceived(const String& data);
    virtual LVoid onStatusCode(LInt statusCode);
    virtual LVoid onFileLen(LInt len);
    virtual LVoid onRedirectUrl(const String& redirectUrl);
    virtual LVoid onLoadError(LInt error);
    virtual LVoid onLoadFinished(const String& data);

private:
	Loader m_loader;
};
}

#endif
