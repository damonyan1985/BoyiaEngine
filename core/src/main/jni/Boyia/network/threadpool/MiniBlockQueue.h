#ifndef BlockQueue_h
#define BlockQueue_h

#include "MiniTaskBase.h"
#include "KList.h"
#include "BoyiaPtr.h"
#include "MiniMutex.h"

namespace yanbo
{
class MiniBlockQueue : public BoyiaRef
{
public:
	MiniBlockQueue();
	~MiniBlockQueue();

	void addTask(MiniTaskBase* task);
	BoyiaPtr<MiniTaskBase> pollTask();
	void clear();
	int size();

private:
	KList<BoyiaPtr<MiniTaskBase> > m_list;
	MiniMutex m_queueMutex;
};
}

#endif
