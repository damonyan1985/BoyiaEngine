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
	};

	UIViewThread();
	static UIViewThread* instance();

	virtual void handleMessage(MiniMessage* msg);

	void destroy();
	void load(const String& url);
	void loadFinished(const String& data, LInt callback);
    void loadError(LInt callback, LInt error);
    void imageLoaded(LInt item);
	void handleKeyEvent(LKeyEvent* evt);
	void draw(LInt item);

private:
    static UIViewThread*    s_instance;
};
}

#endif
