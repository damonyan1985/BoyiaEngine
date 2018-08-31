#include "JSEventHandler.h"
#include "MiniLib.h"
#include "SalLog.h"

namespace mjs
{
JSEventHandler::JSEventHandler()
    : m_touchCallback(NULL)
    , m_keyCallback(NULL)
{
}

JSEventHandler::~JSEventHandler()
{
	if (NULL != m_touchCallback)
	{
		delete m_touchCallback;
	}

	if (NULL != m_keyCallback)
	{
		delete m_keyCallback;
	}
}

LVoid JSEventHandler::setTouchCallback(MiniValue* callback)
{
	m_touchCallback = new MiniValue;
	ValueCopy(m_touchCallback, callback);
}

LVoid JSEventHandler::handleTouch(const util::LTouchEvent& evt)
{
	if (!m_touchCallback)
		return;
	KFORMATLOG("JSEventHandler::handleTouch %d", 1);
	SaveLocalSize();
	LocalPush(m_touchCallback);

	MiniValue val;
	val.mValueType = M_INT;
	val.mValue.mIntVal = evt.getType();
	LocalPush(&val);

	val.mValueType = M_INT;
	val.mValue.mIntVal = evt.getPosition().iX;
	LocalPush(&val);

	val.mValueType = M_INT;
	val.mValue.mIntVal = evt.getPosition().iY;
	LocalPush(&val);
	NativeCall(NULL);
}

LVoid JSEventHandler::setKeyCallback(MiniValue* callback)
{
	m_keyCallback = new MiniValue;
	ValueCopy(m_keyCallback, callback);
}

LVoid JSEventHandler::handleKey(const util::LKeyEvent& evt)
{

}

}
