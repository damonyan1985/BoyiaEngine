/*
 * LoaderWin.h
 *
 *  Created on: 2019-11-16
 *      Author: yanbo
 */

#ifndef LoaderWin_h
#define LoaderWin_h

#include "NetworkBase.h"

#if ENABLE(BOYIA_WINDOWS)
#include "UtilString.h"

namespace yanbo {
class LoaderWin : public NetworkBase {
public:
    LoaderWin();
    virtual ~LoaderWin();

public:
    virtual LVoid syncLoadUrl(const String& url, String& content);
    virtual LVoid loadUrl(const String& url, NetworkClient* client);
    virtual LVoid loadUrl(const String& url, NetworkClient* client, LBool isWait);

    virtual LVoid postData(const String& url, NetworkClient* client);
    virtual LVoid postData(const String& url, NetworkClient* client, LBool isWait);
    virtual LVoid cancel();
};
}

#endif

#endif