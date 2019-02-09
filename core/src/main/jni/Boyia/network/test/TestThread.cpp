#include "MiniThread.h"
#include "SalLog.h"
#include "SystemUtil.h"

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
        	//waitTimeOut(1000);
            long now = SystemUtil::getSystemTime();
            sleep(1000);
        	//KLOG("waitTimeOut 1000");
            KFORMATLOG("TimeThread sleepTime=%ld", SystemUtil::getSystemTime() - now);
        }
    }	
};
}

extern LVoid TestThread()
{
	yanbo::TimeThread* t = new yanbo::TimeThread;
	t->start();
}