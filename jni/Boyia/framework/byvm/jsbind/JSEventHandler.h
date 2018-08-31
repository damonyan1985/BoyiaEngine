#ifndef JSEventHandler_h
#define JSEventHandler_h

#include "MiniCore.h"
#include "LEvent.h"

namespace mjs
{
class JSEventHandler
{
public:
	JSEventHandler();
	~JSEventHandler();

	LVoid setTouchCallback(MiniValue* callback);
	LVoid handleTouch(const util::LTouchEvent& evt);

	LVoid setKeyCallback(MiniValue* callback);
	LVoid handleKey(const util::LKeyEvent& evt);

private:
	MiniValue* m_touchCallback;
	MiniValue* m_keyCallback;
};
}

#endif
