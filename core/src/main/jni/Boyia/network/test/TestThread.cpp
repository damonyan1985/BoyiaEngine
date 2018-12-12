#include "MiniThread.h"
#include "SalLog.h"

namespace yanbo
{
class TimeThread : public MiniThread
{
public:
    void run() 
    {
        while (true)
        {
        	//KLOG("waitTimeOut TimeThread begin");
        	waitTimeOut(1000);
        	KLOG("waitTimeOut 1000");
        }
    }	
};
}

extern LVoid TestThread()
{
	yanbo::TimeThread* t = new yanbo::TimeThread;
	t->start();
}