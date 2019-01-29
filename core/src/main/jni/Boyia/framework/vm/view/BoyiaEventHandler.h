#ifndef BoyiaEventHandler_h
#define BoyiaEventHandler_h

#include "BoyiaCore.h"
#include "LEvent.h"

namespace boyia
{
class BoyiaEventHandler
{
public:
	BoyiaEventHandler();
	~BoyiaEventHandler();

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
