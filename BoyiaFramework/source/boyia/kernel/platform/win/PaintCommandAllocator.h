#ifndef PaintCommandAllocator_h
#define PaintCommandAllocator_h

#include "LGdi.h"
#include "UtilString.h"
#include <windows.h>
#include <GdiPlus.h>

namespace util {
class PaintResource;
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
    Bool inUse;
    PaintResource* resource;
};

class PaintResource {
public:
    virtual ~PaintResource() {}
    virtual LInt type() const = 0;

    LRgb color;
    LRect rect;
};

class PaintImageResource : public PaintResource {
public:
    PaintImageResource()
        : image(kBoyiaNull)
    {
    }

    virtual LInt type() const 
    {
        return PaintCommand::kPaintImage;
    }

    Gdiplus::Image* image;
};

class PaintRectResource : public PaintResource {
public:
    virtual LInt type() const
    {
        return PaintCommand::kPaintRect;
    }
};

class PaintTextResource : public PaintResource {
public:
    virtual LInt type() const
    {
        return PaintCommand::kPaintText;
    }

    LFont font;
    String text;
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