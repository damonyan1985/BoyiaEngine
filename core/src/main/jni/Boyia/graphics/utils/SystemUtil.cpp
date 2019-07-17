#include "SystemUtil.h"
#include <time.h>

namespace yanbo {
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

    long f = nowTimeval.tv_sec * 1000 + (nowTimeval.tv_usec / 1000);
    return f;
}

long SystemUtil::getSystemMicroTime()
{
    struct timeval nowTimeval;
    gettimeofday(&nowTimeval, NULL);
    long f = nowTimeval.tv_sec * 1000 * 1000 + nowTimeval.tv_usec;
    return f;
}

int SystemUtil::intCeil(int dividend, int divid)
{
    return dividend % divid > 0 ? (dividend / divid + 1) : (dividend / divid);
}

bool SystemUtil::timeIsBetween(LUint32 later, LUint32 middle, LUint32 earlier)
{
    if (earlier <= later) {
        return ((earlier <= middle) && (middle <= later));
    } else {
        return !((later < middle) && (middle < earlier));
    }
}

LInt32 SystemUtil::timeDiff(LUint32 later, LUint32 earlier)
{
    LUint32 LAST = 0xFFFFFFFF;
    LUint32 HALF = 0x80000000;
    if (timeIsBetween(earlier + HALF, later, earlier)) {
        if (earlier <= later) {
            return static_cast<long>(later - earlier);
        } else {
            return static_cast<long>(later + (LAST - earlier) + 1);
        }
    } else {
        if (later <= earlier) {
            return -static_cast<long>(earlier - later);
        } else {
            return -static_cast<long>(earlier + (LAST - later) + 1);
        }
    }
}
}
