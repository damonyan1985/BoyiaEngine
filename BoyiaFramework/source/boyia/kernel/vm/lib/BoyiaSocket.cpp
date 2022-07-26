#include "BoyiaSocket.h"
#include "UIThread.h"
#include "SalLog.h"

namespace boyia {
BoyiaSocket::BoyiaSocket(const String& url, BoyiaValue* msgCB, BoyiaRuntime* runtime)
    : BoyiaBase(runtime)
    , m_wsUrl(url)
    , m_socket(kBoyiaNull)
{
    if (msgCB != kBoyiaNull) {
        ValueCopy(&m_msgCB, msgCB);
    } else {
        m_msgCB.mValue.mObj.mPtr = kBoyiaNull;
    }
    
    ref();
    start();
}

LVoid BoyiaSocket::onListen()
{
    for (LInt i = 0; i < m_msgs.size(); i++) {
        m_socket->send(m_msgs[i]);
    }
}

LVoid BoyiaSocket::send(const String& message)
{
    if (m_socket) {
        m_socket->send(message);
    } else {
        m_msgs.addElement(message);
    }
}

LVoid BoyiaSocket::handleMessage(const String& message)
{
    BOYIA_LOG(">>> %s", GET_STR(message));

    WeakPtr<BoyiaSocket>* weak = new WeakPtr<BoyiaSocket>(this);
    yanbo::UIThread::instance()->postClosureTask([weak, message] {
        if (weak->expired()) {
            return;
        }
        String msg(message, message.GetLength());
        weak->get()->onMessage(msg);
        msg.ReleaseBuffer();
        delete weak;
    });
}

LVoid BoyiaSocket::onMessage(const String& message)
{
    if (!m_msgCB.mValue.mObj.mPtr) {
        return;
    }
    
    BoyiaValue msg;
    CreateNativeString(&msg,
        (LInt8*)message.GetBuffer(), message.GetLength(), m_runtime->vm());
    // 保存当前栈
    SaveLocalSize(m_runtime->vm());
    // callback函数压栈
    LocalPush(&m_msgCB, m_runtime->vm());
    LocalPush(&msg, m_runtime->vm());
    
    BoyiaValue obj;
    obj.mValueType = BY_CLASS;
    obj.mValue.mObj.mPtr = m_msgCB.mValue.mObj.mSuper;
    
    NativeCall(&obj, m_runtime->vm());
}

LVoid BoyiaSocket::run()
{
    yanbo::WebSocket* socket = yanbo::WebSocket::create(m_wsUrl);
    if (!socket) {
        return;
    }

    socket->setHandler(this);
    //onListen();
    // 切到UI线程处理消息
    WeakPtr<BoyiaSocket>* weak = new WeakPtr<BoyiaSocket>(this);
    yanbo::UIThread::instance()->postClosureTask([weak]() -> void {
        if (weak->expired()) {
            return;
        }
        weak->get()->onListen();
        delete weak;
    });
    
    m_socket = socket;

    while (m_socket->getReadyState() != yanbo::WebSocket::kClosed) {
        // 接受消息
        m_socket->poll();
        // 派发消息
        m_socket->dispatch();
    }
    
    delete this;
}

LVoid BoyiaSocket::release()
{
    onlyDeref();
    m_socket->close();
}

BoyiaSocket::~BoyiaSocket()
{
}
}
