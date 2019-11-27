#ifndef PaintCommandAllocator_h
#define PaintCommandAllocator_h

#include "LGdi.h"
#include "UtilString.h"

namespace util {
class PaintCommand {
public:
    enum PaintType
    {
        kPaintRect,
        kPaintImage,
        kPaintCircle,
        kPaintText
    };

    LRgb color;
    String text;
    LRect rect;
    LInt type;
    Bool inUse;
};

class PaintCommandAllocator {
public:
    PaintCommandAllocator();
    ~PaintCommandAllocator();

    LVoid initAllocator();
    PaintCommand* alloc() const;

private:
    PaintCommand* m_cmds;
};
}

#endif