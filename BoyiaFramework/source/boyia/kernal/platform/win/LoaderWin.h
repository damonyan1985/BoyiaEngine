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
#include "BoyiaHttpEngine.h"
#include "MessageThread.h"

namespace yanbo {
class LoaderWin : public NetworkBase, public MessageThread {
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

    virtual LVoid handleMessage(Message* msg);
};
}

#endif

#endif