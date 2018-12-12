#ifndef MiniMessageThread_h
#define MiniMessageThread_h

#include "MiniMessageQueue.h"
#include "MiniThread.h"

namespace yanbo
{
class MiniMessageThread : public MiniThread
{
public:
	MiniMessageThread();

	virtual void handleMessage(MiniMessage* msg) = 0;
	void postMessage(MiniMessage* msg);
	MiniMessage* obtain();

protected:
	virtual void run();
	MiniMessageQueue*  m_queue;
	LBool              m_continue;
};
}

#endif
