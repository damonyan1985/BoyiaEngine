#ifndef UIThreadClientMap_h
#define UIThreadClientMap_h

#include "KVector.h"

namespace yanbo {

class UIThreadClient {
public:
    UIThreadClient();
    virtual ~UIThreadClient();

    virtual LVoid onClientCallback() = 0;
    LVoid setClientId(LInt id);
    LInt getClientId() const;

private:
    LInt m_clientId;
};

class UIThreadItem;
class UIThreadClientMap {
public:
    static UIThreadClientMap* instance();
    LVoid registerClient(UIThreadClient* client);
    KVector<UIThreadItem*>& map();
    LVoid removeItem(LInt id);
    LVoid clientCallback(LInt id);

private:
    UIThreadClientMap();
    UIThreadItem* getUIThreadItem(LInt id);

    KVector<UIThreadItem*> m_map;
};
}

#endif