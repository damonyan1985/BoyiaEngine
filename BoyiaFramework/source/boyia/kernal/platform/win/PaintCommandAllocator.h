#ifndef PaintCommandAllocator_h
#define PaintCommandAllocator_h

#include "LGdi.h"
#include "UtilString.h"
#include <windows.h>
#include <GdiPlus.h>

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

    PaintCommand();
    ~PaintCommand();
   
    LVoid paint(Gdiplus::Graphics& gc);

    LRgb color;
    LFont font;
    String text;
    
    LRect rect;
    LInt type;
    Gdiplus::Image* image;
    Bool inUse;
};

class PaintCommandAllocator {
public:
    PaintCommandAllocator();
    ~PaintCommandAllocator();

    static PaintCommandAllocator* instance();

    LVoid initAllocator();
    PaintCommand* alloc() const;

private:
    PaintCommand* m_cmds;
};
}

#endif