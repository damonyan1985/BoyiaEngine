#ifndef MiniTaskThread_h
#define MiniTaskThread_h

#include "MiniThread.h"
#include "MiniBlockQueue.h"

namespace yanbo
{
class MiniTaskThread : public MiniThread, public KRef
{
public:
	MiniTaskThread(MiniBlockQueue* queue);
	~MiniTaskThread();

	virtual void stop();
	LBool working();

protected:
	virtual void run();

private:
	MiniBlockQueue* m_queue;
	LBool m_continue;
	LBool m_working;
};

}
#endif
