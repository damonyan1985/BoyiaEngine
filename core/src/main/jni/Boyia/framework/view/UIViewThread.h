#ifndef UIViewThread_h
#define UIViewThread_h

#include "UtilString.h"
#include "MiniMessageThread.h"
#include "LEvent.h"
#include "GLContext.h"

namespace yanbo
{
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
	};

	UIViewThread();
	static UIViewThread* instance();

	virtual void handleMessage(MiniMessage* msg);

	void destroy();
	void load(const String& url);
	void dataReceived(LByte* bytes, LInt len, LIntPtr callback);
	void loadFinished(LIntPtr callback);
    void loadError(LIntPtr callback, LInt error);
    void imageLoaded(LIntPtr item);
	void handleKeyEvent(LKeyEvent* evt);
	void draw(LIntPtr item);

private:
    static UIViewThread*    s_instance;
};
}

#endif
