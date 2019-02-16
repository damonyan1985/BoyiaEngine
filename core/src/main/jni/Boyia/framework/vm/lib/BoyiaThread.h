#ifndef UIViewThread_h
#define UIViewThread_h

#include "UtilString.h"
#include "MiniMessageThread.h"
#include "LEvent.h"
#include "GLContext.h"

namespace yanbo
{
class BoyiaEvent
{
public:
    virtual ~BoyiaEvent();

private:
	LVoid execute();
    
protected:
    virtual LVoid run() = 0;
    friend class BoyiaThread;
};

class BoyiaThread : public MiniMessageThread
{
public:
	enum Operation
	{
        BOYIA_INIT = 1,
        BOYIA_QUIT,
	    BOYIA_SEND_EVENT,
	};

	BoyiaThread();
	static BoyiaThread* instance();

	virtual LVoid handleMessage(MiniMessage* msg);
	LVoid destroy();
	LVoid load(const String& url);
	LVoid sendEvent(BoyiaEvent* event);

private:
    static BoyiaThread*    s_instance;
};
}

#endif
