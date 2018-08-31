#include "MiniMessageThread.h"

namespace yanbo
{
MiniMessageThread::MiniMessageThread()
    : m_queue(new MiniMessageQueue())
    , m_continue(LTrue)
{
}

LVoid MiniMessageThread::postMessage(MiniMessage* msg)
{
	m_queue->push(msg);
	notify();
}

MiniMessage* MiniMessageThread::obtain()
{
	return m_queue->obtain();
}

void MiniMessageThread::run()
{
	while (m_continue)
	{
		if (m_queue == NULL)
		{
		    return;
		}

		MiniMessage* msg = m_queue->poll();
	    if (msg != NULL)
	    {
	    	handleMessage(msg);
	    	msg->msgRecycle();
	    }
	    else
	    {
	    	MiniThread::waitOnNotify();
	    }
	}
}

}
