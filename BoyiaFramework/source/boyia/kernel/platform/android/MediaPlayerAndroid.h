#ifndef MediaPlayerAndroid_h
#define MediaPlayerAndroid_h

#include "LGdi.h"
#include "UIThreadClientMap.h"

namespace util {

struct JMediaPlayer;
class MediaPlayerAndroid : public LMediaPlayer, public yanbo::UIThreadClient {
public:
    MediaPlayerAndroid(LVoid* view);
    ~MediaPlayerAndroid();

public:
    virtual LVoid start(const String& url);
    virtual LVoid pause();
    virtual LVoid stop();
    virtual LVoid seek(LInt progress);
    virtual LVoid onClientCallback();

private:
    struct JMediaPlayer* m_player;
    LVoid* m_view;
};
}

#endif
