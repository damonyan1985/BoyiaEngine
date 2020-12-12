#include "BoyiaAsyncEvent.h"
#include "KVector.h"

namespace boyia {
class BoyiaAsyncMapTable {
public:
    struct EventMapEntry {
        EventMapEntry(LIntPtr ptr)
            : objPtr(ptr)
        {
        }
        LIntPtr objPtr;
        BoyiaList<BoyiaAsyncEvent*> eventList;
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
            entry->eventList.push(event);
        } else {
            EventMapEntry* entry = new EventMapEntry(
                event->m_obj.mValue.mObj.mPtr);

            m_eventMap.addElement(entry);
        }

    }

    LBool hasObject(BoyiaValue* obj)
    {
        for (LInt i = 0; i < m_eventMap.size(); i++) {
            if (obj->mValue.mObj.mPtr == m_eventMap[i]->objPtr) {
                return LTrue;
            }
        }

        return LFalse;
    }

    LVoid removeAllEvent(LIntPtr ptr) {
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

        EventMapEntry* entry = m_eventMap[index].get();
        BoyiaList<BoyiaAsyncEvent*>::Iterator iter = entry->eventList.begin();
        BoyiaList<BoyiaAsyncEvent*>::Iterator end = entry->eventList.end();
        for (; iter != end; ++iter) {
            if (event == *iter) {
                entry->eventList.erase(iter);
                break;
            }
        }

        if (!entry->eventList.count()) {
            m_eventMap.remove(index);
        }
    }

private:
    KVector<OwnerPtr<EventMapEntry> > m_eventMap;
};

BoyiaAsyncMapTable BoyiaAsyncEvent::s_table;
LVoid BoyiaAsyncEvent::registerEvent(BoyiaAsyncEvent* event)
{
    return s_table.registerEvent(event);
}

LBool BoyiaAsyncEvent::hasObject(BoyiaValue* obj)
{
    return s_table.hasObject(obj);
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
    if (!BoyiaAsyncEvent::hasObject(&m_obj)) {
        return;
    }

    callback();
    // Remove this on UI thread
    BoyiaAsyncEvent::removeEvent(this);
}
}