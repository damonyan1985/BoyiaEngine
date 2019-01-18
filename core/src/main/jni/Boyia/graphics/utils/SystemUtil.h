#ifndef FunshionUtil_h
#define FunshionUtil_h

#include "PlatformLib.h"

namespace yanbo
{
class SystemUtil
{
public:
	static long getSystemTime();
	static int intCeil(int dividend, int divid);
	bool timeIsBetween(LUint32 later, LUint32 middle, LUint32 earlier);
	LInt32 timeDiff(LUint32 later, LUint32 earlier);
};
}
#endif
