#include "BoyiaOpcode.h"

namespace boyia {
Condition ReverseCondition(Condition cond)
{
	switch (cond)
	{
	case lo:
		return hi;
	case hi:
		return lo;
	case hs:
		return ls;
	case ls:
		return hs;
	case lt:
		return gt;
	case gt:
		return lt;
	case ge:
		return le;
	case le:
		return ge;
	default:
		return cond;
	};
}
}
