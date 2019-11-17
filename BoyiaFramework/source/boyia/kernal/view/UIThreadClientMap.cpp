#include "UIThreadClientMap.h"
#include "StringBuilder.h"

namespace yanbo {

class UIThreadItem {
public:
    UIThreadItem()
        : client(NULL)
        , id(0)
        , next(NULL)
    {
    }

    UIThreadClient* client;
    LInt id;
    UIThreadItem* next;
};

UIThreadClient::UIThreadClient()
    : m_clientId(0)
{
}

UIThreadClient::~UIThreadClient()
{
    UIThreadClientMap::instance()->removeItem(m_clientId);
}

LVoid UIThreadClient::setClientId(LInt id)
{
    m_clientId = id;
}

LInt UIThreadClient::getClientId() const
{
    return m_clientId;
}

// only call in ui thread
static LInt sUIThreadClienId = 0;
static LInt UIThreadClienIdCreate()
{
    return ++sUIThreadClienId;
}

// create a size and capactiy vector
UIThreadClientMap::UIThreadClientMap()
    : m_map(kEnlargeCapacity)
{
    // Init all pointer to NULL
    for (LInt i = 0; i < m_map.capacity(); ++i) {
        m_map[i] = NULL;
    }
}

LVoid UIThreadClientMap::clientCallback(LInt id)
{
    KFORMATLOG("UIThreadClientMap::clientCallback clientId=%d", id);
    UIThreadItem* item = getUIThreadItem(id);
    if (item) {
        KFORMATLOG("UIThreadClientMap::clientCallback item->client=%ld", (long)item->client);
        item->client->onClientCallback();
        delete item;
    }
}

UIThreadClientMap* UIThreadClientMap::instance()
{
    static UIThreadClientMap sUIThreadClientMap;
    return &sUIThreadClientMap;
}

KVector<UIThreadItem*>& UIThreadClientMap::map()
{
    return m_map;
}

UIThreadItem* UIThreadClientMap::getUIThreadItem(LInt id)
{
    LInt index = id % m_map.capacity();
    UIThreadItem* item = m_map[index];
    KFORMATLOG("UIThreadClientMap::getUIThreadItem item=%ld", (long)item);
    // Item maybe delete by uithread before this even run
    if (!item || !item->client) {
        return NULL;
    }

    KFORMATLOG("UIThreadClientMap::getUIThreadItem item->id=%d", item->id);
    if (item->id == id) {
        m_map[index] = item->next;
        return item;
    } else {
        UIThreadItem* next = item->next;
        while (next) {
            KFORMATLOG("UIThreadClientMap::getUIThreadItem next->id=%d", next->id);
            if (next->id == id) {
                item->next = next->next;
                return next;
            }

            item = next;
            next = next->next;
        }
    }
}

LVoid UIThreadClientMap::removeItem(LInt id)
{
    UIThreadItem* item = getUIThreadItem(id);
    if (item) {
        delete item;
    }
}

// Only use in ui thread
LVoid UIThreadClientMap::registerClient(UIThreadClient* client)
{
    KFORMATLOG("UIThreadClientMap::registerClient client=%ld", (long)client);
    LInt id = UIThreadClienIdCreate();
    UIThreadItem* item = new UIThreadItem();

    KFORMATLOG("UIThreadClientMap::registerClient id=%d", (long)id);
    client->setClientId(id);
    item->id = id;
    item->client = client;
    //item->m_map.addElement(item);
    LInt index = id % m_map.capacity();
    UIThreadItem* mapItem = m_map[index];
    m_map[index] = item;
    if (mapItem) {
        item->next = mapItem;
    }
}
}