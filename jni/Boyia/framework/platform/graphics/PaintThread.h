#ifndef PaintThread_h
#define PaintThread_h

#include "KRef.h"
#include "LGdi.h"
#include "MiniMessageThread.h"
#include "GLContext.h"
#include "LEvent.h"

namespace yanbo
{
class GraphicsContextGL;
class PaintThread : public MiniMessageThread
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
		UI_SETINPUT,
		UI_VIDEO_UPDATE,
		UI_OP_EXEC,
		UI_DESTROY,
	};
	static PaintThread* instance();

	virtual ~PaintThread();
	LVoid setGC(LGraphicsContext* gc);

	LVoid initContext(LVoid* win);
	LVoid resetContext(LVoid* win);

	LVoid draw(LVoid* item);
	LVoid drawOnly(LVoid* item);
	LVoid submit();
	// EditText渲染
	LVoid setInputText(const String& text, LInt item);
	// Video渲染
	LVoid videoUpdate(LIntPtr item);
	LVoid destroy();
	LVoid handleTouchEvent(LTouchEvent* evt);
	LVoid uiExecute();
	virtual LVoid handleMessage(MiniMessage* msg);

private:
	PaintThread();
	LVoid initGL();
	LVoid resetGL();
	LVoid flush();

	LVoid drawUI(LVoid* view);
	LGraphicsContext*            m_gc;
	util::GLContext              m_context;
	static PaintThread*          s_inst;
};
}
#endif
