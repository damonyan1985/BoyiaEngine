#include "UIThreadClientMap.h"
#include "StringBuilder.h"

namespace yanbo {

class UIThreadItem {
public:
    UIThreadItem()
        : client(kBoyiaNull)
        , id(0)
        , next(kBoyiaNull)
    {
    }

    UIThreadClient* client;
    LInt id;
    UIThreadItem* next;
};

UIThreadClient::UIThreadClient()
    : m_clientId(0)
{
    yanbo::UIThreadClientMap::instance()->registerClient(this);
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
    : m_map(kEnlargeCapacity * 4)
{
    // Init all pointer to NULL
    for (LInt i = 0; i < m_map.capacity(); ++i) {
        m_map[i] = kBoyiaNull;
    }
}

LVoid UIThreadClientMap::clientCallback(LInt id)
{
    KFORMATLOG("UIThreadClientMap::clientCallback clientId=%d", id);
    UIThreadClient* client = findUIThreadClient(id);
    if (client) {
        KFORMATLOG("UIThreadClientMap::clientCallback item->client=%ld", (long)client);
        client->onClientCallback();
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

// UIThreadClient* UIThreadClientMap::getUIThreadClient(LInt id)
// {
//     UIThreadItem* item = getUIThreadItem(id);
//     return item ? item->client : kBoyiaNull;
// }

UIThreadClient* UIThreadClientMap::findUIThreadClient(LInt id)
{
    // LInt index = id % m_map.capacity();
    // (n - 1) & hash = hash % n
    LInt index = (m_map.capacity() - 1) & id;
    UIThreadItem* item = m_map[index];
    while (item) {
        if (item->id == id) {
            return item->client;
        }

        item = item->next;
    }

    KFORMATLOG("UIThreadClientMap::findUIThreadClient item=%ld", (long)item);
    KFORMATLOG("UIThreadClientMap::findUIThreadClient item->id=%d", item->id);
    return item->client;
}

UIThreadItem* UIThreadClientMap::getUIThreadItem(LInt id)
{
    // LInt index = id % m_map.capacity();
    // (n - 1) & hash = hash % n
    LInt index = (m_map.capacity() - 1) & id;
    UIThreadItem* item = m_map[index];
    KFORMATLOG("UIThreadClientMap::getUIThreadItem item=%ld", (long)item);
    // Item maybe delete by uithread before this even run
    if (!item || !item->client) {
        return kBoyiaNull;
    }

    KFORMATLOG("UIThreadClientMap::getUIThreadItem item->id=%d", item->id);
    if (item->id == id) {
        m_map[index] = item->next;
        return item;
    }

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
    
    return kBoyiaNull;
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
    // LInt index = id % m_map.capacity();
    // (n - 1) & hash = hash % n
    LInt index = (m_map.capacity() - 1) & id;
    UIThreadItem* mapItem = m_map[index];
    m_map[index] = item;
    if (mapItem) {
        item->next = mapItem;
    }
}
}
