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
#include "TaskBase.h"
#include "FileUtil.h"
#include "PlatformBridge.h"
#include "OwnerPtr.h"

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

class FileTask : public TaskBase {
public:
    FileTask(NetworkClient* client)
        : m_client(client)
    {
    }

    LVoid setUrl(const String& url)
    {
        // shallow copy
        m_url.Copy(url.GetBuffer(), LTrue, url.GetLength());
    }

    virtual LVoid execute()
    {
        BOYIA_LOG("FileTask---execute()---%d", 0);
        if (m_client) {
            String content;
            BOYIA_LOG("FileTask---execute()---%s", GET_STR(m_url));
            FileUtil::readFile(m_url, content);
            BOYIA_LOG("FileTask---execute()---%s", GET_STR(content));

            m_client->onDataReceived(content.GetBuffer(), content.GetLength());
            BOYIA_LOG("FileTask---execute()---%d", 3);
            m_client->onLoadFinished();
            BOYIA_LOG("FileTask---execute()---%d", 4);
        }
    }

private:
    String m_url;
    NetworkClient* m_client;
};

static FileTask* loadBoyiaUrl(const String& url, NetworkClient* client)
{
    const String& schema = FileUtil::fileSchema();
    if (url.StartWith(schema)) {
        String sourceUrl = url.Mid(schema.GetLength());
        String sourcePath = _CS(PlatformBridge::getAppRoot()) + sourceUrl;
        FileTask* fileTask = new FileTask(client);
        fileTask->setUrl(sourcePath);
        sourcePath.ReleaseBuffer();
        return fileTask;
    }
    
    return kBoyiaNull;
}

// TODO
class LoaderIOS : public NetworkBase, public MessageThread {
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
        // schema如果是boyia://, 则进行文件请求
        FileTask* task = loadBoyiaUrl(url, client);
        if (task) {
            Message* msg = m_queue->obtain();
            msg->type = kBoyiaNull;
            msg->obj = task;
            m_queue->push(msg);
            notify();
            return;
        }
        
        // 网络请求
        [m_engine loadUrlWithData:kHttpGet url:GET_STR(url) callback:[[LoaderClient alloc] initWithClient:client]];
    }
    
    virtual LVoid loadUrl(const String& url, NetworkClient* client, LBool isWait) {}

    virtual LVoid postData(const String& url, NetworkClient* client) {}
    virtual LVoid postData(const String& url, NetworkClient* client, LBool isWait) {}
    virtual LVoid cancel() {};
    
    // 只处理文件加载
    virtual LVoid handleMessage(Message* msg)
    {
        OwnerPtr<FileTask> task = static_cast<FileTask*>(msg->obj);
        if (task) {
            task->execute();
        }
    }
    
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
