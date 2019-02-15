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
private:
	LVoid execute();
    
protected:
    virtual LVoid run() = 0;
    friend class UIViewThread;
};

class UIViewThread : public MiniMessageThread
{
public:
	enum UIViewOperation
	{
        UIView_INIT = 1,
        UIView_DRAW,
        UIView_QUIT,
	    UIView_KEYEVENT,
	    UIView_LOAD_FINISHED,
	    UIView_LOAD_ERROR,
	    UIView_IMAGE_LOADED,
	    UIView_TOUCH_EVENT,
	    UIView_DATA_RECV,
	    UIView_SEND_EVENT,
	};

	UIViewThread();
	static UIViewThread* instance();

	virtual LVoid handleMessage(MiniMessage* msg);

	LVoid destroy();
	LVoid load(const String& url);
	LVoid dataReceived(LByte* bytes, LInt len, LIntPtr callback);
	LVoid loadFinished(LIntPtr callback);
    LVoid loadError(LIntPtr callback, LInt error);
    LVoid imageLoaded(LIntPtr item);
	LVoid handleKeyEvent(LKeyEvent* evt);
	LVoid draw(LIntPtr item);
	LVoid sendEvent(BoyiaEvent* event);

private:
    static UIViewThread*    s_instance;
};
}

#endif
