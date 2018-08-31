#ifndef BlockQueue_h
#define BlockQueue_h

#include "MiniTaskBase.h"
#include "KList.h"
#include "KRefPtr.h"
#include "MiniMutex.h"

namespace yanbo
{
class MiniBlockQueue : public KRef
{
public:
	MiniBlockQueue();
	~MiniBlockQueue();

	void addTask(MiniTaskBase* task);
	KRefPtr<MiniTaskBase> pollTask();
	void clear();
	int size();

private:
	KList<KRefPtr<MiniTaskBase> > m_list;
	MiniMutex m_queueMutex;
};
}

#endif
