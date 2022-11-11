#include "BoyiaAsyncEvent.h"
#include "HashMap.h"
#include "HashUtil.h"
#include "KVector.h"

namespace boyia {
class BoyiaAsyncMapTable {
public:
    struct EventMapEntry {
        EventMapEntry(LIntPtr ptr)
            : objPtr(ptr)
        {
        }

        LBool contains(LInt uniqueId)
        {
            return eventList.get(HashInt(uniqueId)) != kBoyiaNull;
        }

        LVoid put(LInt uniqueId, BoyiaAsyncEvent* event)
        {
            eventList.put(HashInt(uniqueId), event);
        }

        LIntPtr objPtr;
        //BoyiaList<BoyiaAsyncEvent*> eventList;
        HashMap<HashInt, BoyiaAsyncEvent*> eventList;
    };

    BoyiaAsyncMapTable()
        : m_eventMap(0, 20)
    {
    }

    LVoid registerEvent(BoyiaAsyncEvent* event)
    {
        // 一个对象下可能会挂多个事件
        EventMapEntry* entry = kBoyiaNull;
        for (LInt i = 0; i < m_eventMap.size(); i++) {
            if (event->m_obj.mValue.mObj.mPtr == m_eventMap[i]->objPtr) {
                entry = m_eventMap[i].get();
                break;
            }
        }

        if (entry) {
            entry->put(event->m_uniqueId, event);
        } else {
            EventMapEntry* entry = new EventMapEntry(
                event->m_obj.mValue.mObj.mPtr);
            entry->put(event->m_uniqueId, event);

            m_eventMap.addElement(entry);
        }
    }

    LBool hasObject(BoyiaValue* obj, LInt uniqueId)
    {
        for (LInt i = 0; i < m_eventMap.size(); i++) {
            if (obj->mValue.mObj.mPtr == m_eventMap[i]->objPtr
                && m_eventMap[i]->contains(uniqueId)) {
                return LTrue;
            }
        }

        return LFalse;
    }

    LVoid removeAllEvent(LIntPtr ptr)
    {
        for (LInt i = 0; i < m_eventMap.size(); i++) {
            if (ptr == m_eventMap[i]->objPtr) {
                m_eventMap.remove(i);
                return;
            }
        }
    }

    LVoid removeEvent(BoyiaAsyncEvent* event)
    {
        LInt index = -1;
        for (LInt i = 0; i < m_eventMap.size(); i++) {
            if (event->m_obj.mValue.mObj.mPtr == m_eventMap[i]->objPtr) {
                index = i;
                break;
            }
        }

        if (index == -1) {
            return;
        }

        m_eventMap[index]->eventList.remove(event->m_uniqueId);
    }

private:
    KVector<OwnerPtr<EventMapEntry>> m_eventMap;
};

//BoyiaAsyncMapTable BoyiaAsyncEvent::s_table;
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
    if (!callback->mValue.mObj.mPtr) {
        return;
    }
    
    BoyiaValue value;
    CreateNativeString(&value,
        (LInt8*)result.GetBuffer(), result.GetLength(), runtime->vm());
    BOYIA_LOG("BoyiaAsyncEvent::callbackString, data=%s", (const char*)result.GetBuffer());
    // 释放字符串控制权
    result.ReleaseBuffer();
    // 保存当前栈
    SaveLocalSize(runtime->vm());
    // callback函数压栈
    LocalPush(callback, runtime->vm());
    // 参数压栈
    LocalPush(&value, runtime->vm());
    //BoyiaValue* cbObj = obj.mValue.mObj.mPtr == 0 ? kBoyiaNull : &obj;
    BoyiaValue obj;
    obj.mValueType = BY_CLASS;
    obj.mValue.mObj.mPtr = callback->mValue.mObj.mSuper;
    // 调用callback函数
    NativeCall(&obj, runtime->vm());
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
}
}
