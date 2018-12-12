#include "BoyiaEventHandler.h"
#include "BoyiaLib.h"
#include "SalLog.h"

namespace boyia
{
BoyiaEventHandler::BoyiaEventHandler()
    : m_touchCallback(NULL)
    , m_keyCallback(NULL)
{
}

BoyiaEventHandler::~BoyiaEventHandler()
{
	if (m_touchCallback)
	{
		delete m_touchCallback;
	}

	if (m_keyCallback)
	{
		delete m_keyCallback;
	}
}

LVoid BoyiaEventHandler::setTouchCallback(BoyiaValue* callback)
{
	m_touchCallback = new BoyiaValue;
	ValueCopy(m_touchCallback, callback);
}

LVoid BoyiaEventHandler::handleTouch(const util::LTouchEvent& evt)
{
	if (!m_touchCallback)
		return;
	KFORMATLOG("BoyiaEventHandler::handleTouch %d", 1);
	SaveLocalSize();
	LocalPush(m_touchCallback);

	BoyiaValue val;
	val.mValueType = BY_INT;
	val.mValue.mIntVal = evt.getType();
	LocalPush(&val);

	val.mValueType = BY_INT;
	val.mValue.mIntVal = evt.getPosition().iX;
	LocalPush(&val);

	val.mValueType = BY_INT;
	val.mValue.mIntVal = evt.getPosition().iY;
	LocalPush(&val);
	NativeCall(NULL);
}

LVoid BoyiaEventHandler::setKeyCallback(BoyiaValue* callback)
{
	m_keyCallback = new BoyiaValue;
	ValueCopy(m_keyCallback, callback);
}

LVoid BoyiaEventHandler::handleKey(const util::LKeyEvent& evt)
{

}

}
