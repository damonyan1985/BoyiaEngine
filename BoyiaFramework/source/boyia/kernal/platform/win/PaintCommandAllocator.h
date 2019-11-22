#ifndef PaintCommandAllocator_h
#define PaintCommandAllocator_h

#include "PlatformLib.h"

namespace util {
class PaintCommand {
public:
    enum PaintType
    {
        kPaintRect,
        kPaintImage,
        kPaintCircle
    };

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