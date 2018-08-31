#ifndef MiniMessage_h
#define MiniMessage_h

#include "KList.h"
#include "AutoLock.h"

namespace yanbo
{
class MiniMessage
{
public:
	MiniMessage();
	LVoid msgRecycle();

public:
    LInt    type;
    LVoid*  obj;
    LIntPtr arg0;
    LIntPtr arg1;
    LBool   recycle;
    LBool   inCache;
};

class MiniMessageCache
{
public:
	MiniMessageCache();
	virtual MiniMessage* obtain();
	LVoid initCache();

private:
	MiniMessage* m_cache;
};

typedef KList<MiniMessage*> MiniMessageList;
class MiniMessageQueue : public MiniMessageCache
{
public:
	MiniMessage* obtain();

	LVoid initCache();
	LVoid push(MiniMessage* msg);
	LVoid removeMessage(LInt type);
	LInt size();
	MiniMessage* poll();

private:
	MiniMessageList m_list;
	MiniMutex m_queueMutex;
};
}
#endif
