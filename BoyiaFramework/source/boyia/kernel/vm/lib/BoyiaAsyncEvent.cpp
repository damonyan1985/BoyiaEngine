#include "BoyiaAsyncEvent.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "KVector.h"

namespace boyia {
class BoyiaAsyncMapTable {
public:
    struct EventMapEntry {
        LBool contains(LInt uniqueId)
        {
            return eventMap.contains(HashInt(uniqueId));
        }

        LVoid put(LInt uniqueId, BoyiaAsyncEvent* event)
        {
            eventMap.put(HashInt(uniqueId), event);
        }

        HashMap<HashInt, BoyiaAsyncEvent*> eventMap;
    };

    BoyiaAsyncMapTable()
    {
    }

    LVoid registerEvent(BoyiaAsyncEvent* event)
    {
        // 一个对象下可能会挂多个事件
        OwnerPtr<EventMapEntry>& entry = m_eventMap[HashPtr(event->m_obj.mValue.mObj.mPtr)];

        if (entry) {
            entry->put(event->m_uniqueId, event);
        } else {
            entry = new EventMapEntry();
            entry->put(event->m_uniqueId, event);
        }
    }

    bool hasObject(BoyiaValue* obj, LInt uniqueId)
    {
        OwnerPtr<EventMapEntry>& entry = m_eventMap[HashPtr(obj->mValue.mObj.mPtr)];
        return entry && entry->contains(uniqueId);
    }

    LVoid removeAllEvent(LIntPtr ptr)
    {

        m_eventMap.remove(HashPtr(ptr));
    }

    LVoid removeEvent(BoyiaAsyncEvent* event)
    {
        OwnerPtr<EventMapEntry>& entry = m_eventMap[HashPtr(event->m_obj.mValue.mObj.mPtr)];
        if (entry) {
            entry->eventMap.remove(event->m_uniqueId);
        }
    }

private:
    HashMap<HashPtr, OwnerPtr<EventMapEntry>> m_eventMap;
};

BoyiaAsyncEventManager::BoyiaAsyncEventManager()
    : m_uniqueId(0)
    , m_table(new BoyiaAsyncMapTable())
{
}

BoyiaAsyncEventManager::~BoyiaAsyncEventManager()
{
    delete m_table;
}

LVoid BoyiaAsyncEventManager::registerEvent(BoyiaAsyncEvent* event)
{
    return m_table->registerEvent(event);
}

LBool BoyiaAsyncEventManager::hasObject(BoyiaValue* obj, LInt uniqueId)
{
    return m_table->hasObject(obj, uniqueId);
}

LVoid BoyiaAsyncEventManager::removeAllEvent(LIntPtr ptr)
{
    m_table->removeAllEvent(ptr);
}

LVoid BoyiaAsyncEventManager::removeEvent(BoyiaAsyncEvent* event)
{
    m_table->removeEvent(event);
}

LInt BoyiaAsyncEventManager::increment()
{
    return ++m_uniqueId;
}

// In UI thread
BoyiaAsyncEvent::BoyiaAsyncEvent(BoyiaValue* obj, BoyiaRuntime* runtime)
    : m_uniqueId(runtime->eventManager()->increment())
    , m_runtime(runtime)
{
    ValueCopy(&m_obj, obj);
    runtime->eventManager()->registerEvent(this);
}

// In UI thread
BoyiaAsyncEvent::~BoyiaAsyncEvent()
{
}

LVoid BoyiaAsyncEvent::callbackString(String& result, BoyiaValue* callback, BoyiaRuntime* runtime)
{
    BoyiaFunction* cbFun = (BoyiaFunction*)callback->mValue.mObj.mPtr;
    if (!cbFun) {
        return;
    }
    
    BoyiaValue value;
    CreateNativeString(&value,
        (LInt8*)result.GetBuffer(), result.GetLength(), runtime->vm());
    BOYIA_LOG("BoyiaAsyncEvent::callbackString, data=%s", (const char*)result.GetBuffer());
    // 释放字符串控制权
    result.ReleaseBuffer();
    // 保存当前栈
    //SaveLocalSize(runtime->vm());
    // callback函数压栈
    //LocalPush(callback, runtime->vm());
    // 参数压栈
    //LocalPush(&value, runtime->vm());
    //BoyiaValue* cbObj = obj.mValue.mObj.mPtr == 0 ? kBoyiaNull : &obj;
    BoyiaValue args[2];
    ValueCopy(&args[0], callback);
    ValueCopy(&args[1], &value);

    // 一些callback函数是由native控制的，这部分函数设计时是手动去栈上拿压栈的参数，而不需要知道参数名，
    // 因而这样的callback函数mParams设计为空，就比如MicroTask的resolve函数
    if (cbFun->mParams) {
        // 设置参数名
        args[1].mNameKey = cbFun->mParams[0].mNameKey;
    }

    BoyiaValue obj;
    obj.mValueType = BY_CLASS;
    obj.mValue.mObj.mPtr = callback->mValue.mObj.mSuper;
    // 调用callback函数
    NativeCallImpl(args, 2, &obj, runtime->vm());
}

// In UI thread
LVoid BoyiaAsyncEvent::run()
{
    // If object is destroyed, dont callback to application
    // 指针地址和uniqueId双保险，确定event的唯一性
    if (!m_runtime->eventManager()->hasObject(&m_obj, m_uniqueId)) {
        return;
    }

    callback();
    // Remove this on UI thread
    m_runtime->eventManager()->removeEvent(this);

    // Consume microtask
    m_runtime->consumeMicroTask();
}
}
