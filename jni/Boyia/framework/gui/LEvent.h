#ifndef LEvent_h
#define LEvent_h

#include "LGraphic.h"

namespace util
{
class LKeyEvent
{
public:
	enum KeyEventType
	{
		KEY_NONE,
		KEY_DOWN,
		KEY_UP,
		KEY_ARROW_DOWN,
		KEY_ARROW_UP,
		KEY_ARROW_LEFT,
		KEY_ARROW_RIGHT,
		KEY_ENTER
	};

public:
    LKeyEvent()
	    : m_eventType(KEY_NONE)
	    , m_repeated(LFalse)
	{
	}
    
    LKeyEvent(KeyEventType type, LBool repeated)
	    : m_eventType(type)
	    , m_repeated(repeated)
	{
	}
    
	~LKeyEvent(){}
	LInt getType() const { return m_eventType; }
	LVoid setType(KeyEventType type) { m_eventType = type; }

	LBool isRepeated() const { return m_repeated; };
private:
	KeyEventType m_eventType;
	LInt m_repeated;   	
};

class LTouchEvent
{
public:
	enum TouchType {
		// Touch Event
	    ETOUCH_DOWN = 1,      // 1
	    ETOUCH_UP   = 1 << 1, // 2
	    ETOUCH_MOVE = 1 << 2, // 4
	};
	LInt   m_type;
	LPoint m_position;

	LInt getType() const
	{
		return m_type;
	}

	const LPoint& getPosition() const
	{
		return m_position;
	}
};

class LMouseEvent
{
public:
	enum MouseEventType
	{
		MOUSE_NONE,
	    MOUSE_PRESS,
	    MOUSE_UP,
	    MOUSE_LONG_PRESS,
	    MOUSE_SCROLL,
	    MOUSE_OVER,
	};
	
public:
	LMouseEvent()
	    : m_eventType(MOUSE_NONE)
	    , m_position(LPoint(0,0))
	{
	}

	LMouseEvent(MouseEventType type, const LPoint& pt)
		: m_eventType(type)
	    , m_position(pt)
	{
	}

	~LMouseEvent(){}
	
	LInt getType() const { return m_eventType; }
	LVoid setType(MouseEventType type) { m_eventType = type; }
	
	const LPoint& getPosition() const { return m_position; }
    LVoid setPostion(const LPoint& pos) { m_position = pos; }
	
private:
	MouseEventType m_eventType;
	LPoint m_position;
};
}

using util::LKeyEvent;
using util::LTouchEvent;
using util::LMouseEvent;
#endif
