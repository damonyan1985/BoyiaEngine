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

BoyiaAsyncMapTable BoyiaAsyncEvent::s_table;
LVoid BoyiaAsyncEvent::registerEvent(BoyiaAsyncEvent* event)
{
    return s_table.registerEvent(event);
}

LBool BoyiaAsyncEvent::hasObject(BoyiaValue* obj, LInt uniqueId)
{
    return s_table.hasObject(obj, uniqueId);
}

LVoid BoyiaAsyncEvent::removeAllEvent(LIntPtr ptr)
{
    s_table.removeAllEvent(ptr);
}

LVoid BoyiaAsyncEvent::removeEvent(BoyiaAsyncEvent* event)
{
    s_table.removeEvent(event);
}

// In UI thread
BoyiaAsyncEvent::BoyiaAsyncEvent(BoyiaValue* obj)
    : m_uniqueId(increment())
{
    ValueCopy(&m_obj, obj);
    BoyiaAsyncEvent::registerEvent(this);
}

// In UI thread
BoyiaAsyncEvent::~BoyiaAsyncEvent()
{
}

// In UI thread
LVoid BoyiaAsyncEvent::run()
{
    // If object is destroyed, dont callback to application
    if (!BoyiaAsyncEvent::hasObject(&m_obj, m_uniqueId)) {
        return;
    }

    callback();
    // Remove this on UI thread
    BoyiaAsyncEvent::removeEvent(this);
}

LInt BoyiaAsyncEvent::increment()
{
    return ++m_uniqueId;
}
}