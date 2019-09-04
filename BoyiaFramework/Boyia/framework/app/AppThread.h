#ifndef AppThread_h
#define AppThread_h

#include "GLContext.h"
#include "LEvent.h"
#include "MessageThread.h"
#include "UtilString.h"

namespace yanbo {
class AppManager;
class AppEvent {
public:
    virtual ~AppEvent();

private:
    LVoid execute();

protected:
    virtual LVoid run() = 0;
    friend class AppThread;
};

class AppThread : public MessageThread {
public:
    enum Operation {
        kAppInit = 1,
        kAppQuit,
        kAppEvent,
    };

    AppThread(AppManager* manager);
    static AppThread* instance();

    virtual LVoid handleMessage(Message* msg);
    LVoid destroy();
    LVoid load(const String& url);
    LVoid sendEvent(AppEvent* event);

private:
    AppManager* m_manager;
};
}

#endif
