#include "SystemUtil.h"
//#include <android/log.h>
#include "SalLog.h"
#if ENABLE(BOYIA_ANDROID)
#include <time.h>
#elif ENABLE(BOYIA_WINDOWS)
#include <sys/timeb.h>
#elif ENABLE(BOYIA_IOS)
#include <sys/time.h>
#endif

extern LVoid BoyiaLog(const char* format, ...);

namespace yanbo {
long SystemUtil::getSystemTime()
{
#if ENABLE(BOYIA_ANDROID) || ENABLE(BOYIA_IOS)
    struct timeval nowTimeval;
    gettimeofday(&nowTimeval, NULL);
    long f = nowTimeval.tv_sec * 1000 + (nowTimeval.tv_usec / 1000);
    return f;
#elif ENABLE(BOYIA_WINDOWS)
    struct timeb rawtime;
    ftime(&rawtime);
    return rawtime.time * 1000 + rawtime.millitm;
#endif
}

long SystemUtil::getSystemMicroTime()
{
#if ENABLE(BOYIA_ANDROID) || ENABLE(BOYIA_IOS)
    struct timeval nowTimeval;
    gettimeofday(&nowTimeval, NULL);
    long f = nowTimeval.tv_sec * 1000 * 1000 + nowTimeval.tv_usec;
    return f;
#elif ENABLE(BOYIA_WINDOWS)
    struct timeb rawtime;
    ftime(&rawtime);
    return (rawtime.time * 1000 + rawtime.millitm) * 1000;
#endif
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

TimeAnalysis::TimeAnalysis(const char* tag)
{
    mTag = tag;
    mTime = SystemUtil::getSystemMicroTime();
}

TimeAnalysis::~TimeAnalysis()
{
    long now = SystemUtil::getSystemMicroTime();
    BOYIA_LOG("%s TimeAnalysis current=%ld now=%ld time=%ld", mTag, mTime, now, now - mTime);
}
}
