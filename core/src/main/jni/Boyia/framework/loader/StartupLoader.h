/**
 * StartupLoader
 * Use For Boyia Apps Download and Load
 * Date: 2018-10-6
 * Author: yanbo
 * All Copyright Reserved
 */
#ifndef AppLoader_h
#define AppLoader_h

#include "BoyiaLoader.h"
#include "KVector.h"
#include <stdio.h>

namespace yanbo
{
class AppInfo;
class StartupLoader : public NetworkClient
{
public:
	StartupLoader();
	LVoid startLoad();

    virtual LVoid onDataReceived(const LByte* data, LInt size);
    virtual LVoid onStatusCode(LInt statusCode);
    virtual LVoid onFileLen(LInt len);
    virtual LVoid onRedirectUrl(const String& redirectUrl);
    virtual LVoid onLoadError(LInt error);
    virtual LVoid onLoadFinished(const String& data);

private:
    LVoid parseConfig();
    LVoid startLoadApp();

	BoyiaLoader m_loader;
	FILE* m_file;
    KVector<AppInfo*> m_appInfos;
};
}

#endif
