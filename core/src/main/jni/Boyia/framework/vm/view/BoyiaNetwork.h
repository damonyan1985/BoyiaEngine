#ifndef BoyiaNetwork_h
#define BoyiaNetwork_h

#include "NetworkBase.h"
#include "BoyiaCore.h"

namespace boyia
{
// BoyiaNetwork会根据回调的情况，自动释放自己
class BoyiaNetwork : public yanbo::NetworkClient
{
public:
	BoyiaNetwork(BoyiaValue* callback, BoyiaValue* obj);
	virtual ~BoyiaNetwork();

public:
	void load(const String& url);

	virtual void onDataReceived(const LByte* data, LInt size);
	virtual void onStatusCode(LInt statusCode);
	virtual void onFileLen(LInt len);
	virtual void onRedirectUrl(const String& redirectUrl);
	virtual void onLoadError(LInt error);
	virtual void onLoadFinished(const String& data);

private:
	BoyiaValue m_callback;
	BoyiaValue m_obj;
};
}
#endif
