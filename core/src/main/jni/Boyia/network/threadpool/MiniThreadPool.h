#ifndef MiniThreadPool_h
#define MiniThreadPool_h

#include "KList.h"
#include "MiniTaskBase.h"
#include "MiniTaskThread.h"

namespace yanbo
{
class MiniThreadPool
{
public:
	static MiniThreadPool* getInstance();
	static void destroy();

	void sendMiniTask(MiniTaskBase* task);

private:
	~MiniThreadPool();
	MiniThreadPool();

	KList<BoyiaPtr<MiniTaskThread> > m_threadList;
	BoyiaPtr<MiniBlockQueue> m_queue;
};
}

#endif
