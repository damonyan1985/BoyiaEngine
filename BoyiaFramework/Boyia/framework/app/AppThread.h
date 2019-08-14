#ifndef AppThread_h
#define AppThread_h

#include "GLContext.h"
#include "LEvent.h"
#include "MiniMessageThread.h"
#include "UtilString.h"

namespace yanbo {
class AppEvent {
public:
    virtual ~AppEvent();

private:
    LVoid execute();

protected:
    virtual LVoid run() = 0;
    friend class AppThread;
};

class AppThread : public MiniMessageThread {
public:
    enum Operation {
        BOYIA_INIT = 1,
        BOYIA_QUIT,
        BOYIA_SEND_EVENT,
    };

    AppThread();
    static AppThread* instance();

    virtual LVoid handleMessage(MiniMessage* msg);
    LVoid destroy();
    LVoid load(const String& url);
    LVoid sendEvent(AppEvent* event);
};
}

#endif
