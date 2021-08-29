/*
 * LoaderWin.h
 *
 *  Created on: 2019-11-16
 *      Author: yanbo
 */

#ifndef LoaderIOS_h
#define LoaderIOS_h

#include "NetworkBase.h"

#if ENABLE(BOYIA_IOS)
#import "HttpEngineIOS.h"

#include "UtilString.h"
#include "MessageThread.h"

@interface LoaderClient : NSObject<HttpCallback>

-(instancetype)initWithClient:(yanbo::NetworkClient*)client;

@end

// 实现LoaderClient
@implementation LoaderClient {
    yanbo::NetworkClient* _client;
}

-(instancetype)initWithClient:(yanbo::NetworkClient*)client {
    self = [super init];
    if (self != nil) {
        _client = client;
    }
    
    return self;
}

-(void)onDataReceive:(NSData*)data {
    if (_client) {
        _client->onDataReceived((const LByte*)data.bytes, (LInt)data.length);
    }
}

-(void)onLoadFinished {
    if (_client) {
        _client->onLoadFinished();
    }
}

@end

namespace yanbo {
// TODO
class LoaderIOS : public NetworkBase{
public:
    LoaderIOS()
    {
        m_engine = [HttpEngineIOS new];
    };
    virtual ~LoaderIOS() {};

public:
    virtual LVoid syncLoadUrl(const String& url, String& content) {};
    
    virtual LVoid loadUrl(const String& url, NetworkClient* client)
    {
        [m_engine loadUrlWithData:kHttpGet url:GET_STR(url) callback:[[LoaderClient alloc] initWithClient:client]];
    }
    
    virtual LVoid loadUrl(const String& url, NetworkClient* client, LBool isWait) {};

    virtual LVoid postData(const String& url, NetworkClient* client) {};
    virtual LVoid postData(const String& url, NetworkClient* client, LBool isWait) {};
    virtual LVoid cancel() {};
    
private:
    HttpEngineIOS* m_engine;
};

NetworkBase* NetworkBase::create()
{
    return new LoaderIOS();
}
}

#endif

#endif
