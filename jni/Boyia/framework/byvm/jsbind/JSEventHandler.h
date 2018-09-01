#ifndef JSEventHandler_h
#define JSEventHandler_h

#include "BoyiaCore.h"
#include "LEvent.h"

namespace boyia
{
class JSEventHandler
{
public:
	JSEventHandler();
	~JSEventHandler();

	LVoid setTouchCallback(BoyiaValue* callback);
	LVoid handleTouch(const util::LTouchEvent& evt);

	LVoid setKeyCallback(BoyiaValue* callback);
	LVoid handleKey(const util::LKeyEvent& evt);

private:
	BoyiaValue* m_touchCallback;
	BoyiaValue* m_keyCallback;
};
}

#endif
