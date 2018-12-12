/*
 * LoaderAndroid.h
 *
 *  Created on: 2014-2-23
 *      Author: yanbo
 */

#ifndef LoaderAndroid_h
#define LoaderAndroid_h

#include "UtilString.h"
#include "NetworkBase.h"
#include <jni.h>

namespace yanbo
{

struct JLoader;
class LoaderAndroid : public NetworkBase
{
public:
	LoaderAndroid();
	virtual ~LoaderAndroid();
	
	void initLoader();

public:
	virtual LVoid loadUrl(const String& url, NetworkClient* client);
	virtual LVoid loadUrl(const String& url, NetworkClient* client, bool isWait);
	virtual LVoid postData(const String& url, NetworkClient* client, bool isWait);
	virtual LVoid cancel();

private:
	void request(const String& url, NetworkClient* client, bool isWait, int method);
	struct JLoader* m_privateLoader;
};

}

#endif /* INPUTSTREAM_H_ */
