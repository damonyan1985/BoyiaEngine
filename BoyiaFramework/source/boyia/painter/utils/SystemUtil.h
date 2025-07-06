#ifndef SystemUtil_h
#define SystemUtil_h

#include "PlatformLib.h"

namespace yanbo {
class SystemUtil {
public:
    // 返回毫秒
    static long getSystemTime();
    // 返回微秒
    static long getSystemMicroTime();
    static int intCeil(int dividend, int divid);
    static bool timeIsBetween(LUint32 later, LUint32 middle, LUint32 earlier);
    static LInt32 timeDiff(LUint32 later, LUint32 earlier);
};

class TimeAnalysis {
public:
    TimeAnalysis(const char* tag);
    ~TimeAnalysis();

private:
    long mTime;
    const char* mTag;
};
}
#endif
