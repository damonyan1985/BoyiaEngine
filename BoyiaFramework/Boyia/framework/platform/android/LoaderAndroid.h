/*
 * LoaderAndroid.h
 *
 *  Created on: 2014-2-23
 *      Author: yanbo
 */

#ifndef LoaderAndroid_h
#define LoaderAndroid_h

#include "NetworkBase.h"
#include "UtilString.h"
#include <jni.h>

namespace yanbo {

struct JLoader;
class LoaderAndroid : public NetworkBase {
public:
    LoaderAndroid();
    virtual ~LoaderAndroid();

    LVoid initLoader();

public:
    virtual LVoid loadUrl(const String& url, NetworkClient* client);
    virtual LVoid loadUrl(const String& url, NetworkClient* client, LBool isWait);

    virtual LVoid postData(const String& url, NetworkClient* client);
    virtual LVoid postData(const String& url, NetworkClient* client, LBool isWait);
    virtual LVoid cancel();

private:
    LVoid request(const String& url, NetworkClient* client, LBool isWait, LInt method);
    struct JLoader* m_privateLoader;

    NetworkMap m_params;
};
}

#endif /* INPUTSTREAM_H_ */
