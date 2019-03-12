#ifndef PaintThread_h
#define PaintThread_h

#include "LGdi.h"
#include "MiniMessageThread.h"
#include "GLContext.h"
#include "LEvent.h"

namespace yanbo
{
class GraphicsContextGL;
class UIThread : public MiniMessageThread
{
public:
	enum MessageType
	{
		UI_INIT = 1,
		UI_RESET,
		UI_DRAW,
		UI_DRAWONLY,
		UI_CREATE_TEX,
		UI_SUBMIT,
		UI_TOUCH_EVENT,
		UI_KEY_EVENT,
		UI_SETINPUT,
		UI_VIDEO_UPDATE,
		UI_IMAGE_LOADED,
		UI_OP_EXEC,
		UI_DESTROY,
	};
	static UIThread* instance();

	virtual ~UIThread();
	LVoid setGC(LGraphicsContext* gc);

	LVoid initContext(LVoid* win);
	LVoid resetContext(LVoid* win);

	LVoid draw(LVoid* item);
	LVoid drawOnly(LVoid* item);
	LVoid submit();
	// EditText渲染
	LVoid setInputText(const String& text, LIntPtr item);
	// Video渲染
	LVoid videoUpdate(LIntPtr item);
	LVoid imageLoaded(LIntPtr item);
	LVoid destroy();
	LVoid handleTouchEvent(LTouchEvent* evt);
	LVoid handleKeyEvent(LKeyEvent* evt);
	LVoid uiExecute();
	virtual LVoid handleMessage(MiniMessage* msg);

private:
	UIThread();
	LVoid initGL();
	LVoid resetGL();
	LVoid flush();

	LVoid drawUI(LVoid* view);
	LGraphicsContext*            m_gc;
	util::GLContext              m_context;
};
}
#endif
