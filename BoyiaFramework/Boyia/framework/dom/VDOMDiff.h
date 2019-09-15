#ifndef VDOMDiff_h
#define VDOMDiff_h

#include "VDocument.h"

namespace yanbo {
class VDOMDiff {
public:
    static LVoid diff(VDocument* oldDom, VDocument* newDom);
};
}

#endif