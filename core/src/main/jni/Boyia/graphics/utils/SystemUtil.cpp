#include "SystemUtil.h"
#include <time.h>

namespace yanbo
{
long SystemUtil::getSystemTime()
{
	struct timeval nowTimeval;
	gettimeofday(&nowTimeval, NULL);
//    struct tm * tm;
//	time_t time_sec ;
//	time_sec = nowTimeval.tv_sec;
//	tm = localtime(&time_sec);

//	nMinute = tm->tm_min;
//	nSecond = tm->tm_sec;
//	nHour = tm->tm_hour;

//	return tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;

	long f = nowTimeval.tv_sec * 1000 + (nowTimeval.tv_usec/1000);
	return f;
}

int SystemUtil::intCeil(int dividend, int divid)
{
    return dividend % divid > 0 ? (dividend/divid + 1) : (dividend/divid);
}

}
