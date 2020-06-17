#include "PaintCommandAllocator.h"
#include "CharConvertor.h"
#include "KVector.h"
#include "PixelRatio.h"

namespace util {
const LInt kPaintCommandDefaultCapacity = 2048;

class PaintCmdHandler {
public:
    virtual LVoid paint(Gdiplus::Graphics& gc, PaintCommand* cmd) = 0;
};

class PaintRectHandler : public PaintCmdHandler {
public:
    virtual LVoid paint(Gdiplus::Graphics& gc, PaintCommand* cmd)
    {
        float ratio = yanbo::PixelRatio::ratio();
        PaintRectResource* resource = static_cast<PaintRectResource*>(cmd->resource);
        float width = resource->rect.GetWidth() * yanbo::PixelRatio::ratio();
        width = width < 1 ? 1 : width;
        float height = resource->rect.GetHeight() * yanbo::PixelRatio::ratio();
        height = height < 1 ? 1 : height;
        Gdiplus::RectF rect(
            resource->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
            resource->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
            width,
            height
        );

        Gdiplus::SolidBrush brush(Gdiplus::Color(
            resource->color.m_alpha,
            resource->color.m_red,
            resource->color.m_green,
            resource->color.m_blue
        ));
        gc.FillRectangle(&brush, rect);
    }
};

class PaintTextHandler : public PaintCmdHandler {
public:
    virtual LVoid paint(Gdiplus::Graphics& gc, PaintCommand* cmd)
    {
        PaintTextResource* resource = static_cast<PaintTextResource*>(cmd->resource);
        wstring wtext = yanbo::CharConvertor::CharToWchar(GET_STR(resource->text));
        Gdiplus::Font font(L"Arial", resource->font.getFontSize() * 0.15);

        Gdiplus::StringFormat format(Gdiplus::StringAlignmentNear);
        Gdiplus::RectF rect(
            resource->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
            resource->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
            resource->rect.GetWidth() * yanbo::PixelRatio::ratio(),
            resource->rect.GetHeight() * yanbo::PixelRatio::ratio()
        );
        
        Gdiplus::SolidBrush brush(Gdiplus::Color(
            resource->color.m_alpha,
            resource->color.m_red,
            resource->color.m_green,
            resource->color.m_blue
        ));

        gc.DrawString(wtext.c_str(), wtext.length(), &font, rect, &format, &brush);
    }
};

class PaintImageHandler : public PaintCmdHandler {
public:
    virtual LVoid paint(Gdiplus::Graphics& gc, PaintCommand* cmd)
    {
        PaintImageResource* resource = static_cast<PaintImageResource*>(cmd->resource);
        Gdiplus::Rect rect(
            resource->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
            resource->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
            resource->rect.GetWidth() * yanbo::PixelRatio::ratio(),
            resource->rect.GetHeight() * yanbo::PixelRatio::ratio()
        );
        gc.DrawImage(resource->image, rect);
    }
};

class PaintCicleHandler : public PaintCmdHandler {
public:
    virtual LVoid paint(Gdiplus::Graphics& gc, PaintCommand* cmd)
    {
    }
};


static KVector<PaintCmdHandler*>* sPaintRegistrayArray = kBoyiaNull;
static LVoid paintFunctionRegister() {
    if (!sPaintRegistrayArray) {
        sPaintRegistrayArray = new KVector<PaintCmdHandler*>(4);
        (*sPaintRegistrayArray)[PaintCommand::kPaintRect] = new PaintRectHandler();
        (*sPaintRegistrayArray)[PaintCommand::kPaintImage] = new PaintImageHandler();
        (*sPaintRegistrayArray)[PaintCommand::kPaintCircle] = new PaintCicleHandler();
        (*sPaintRegistrayArray)[PaintCommand::kPaintText] = new PaintTextHandler();
    }
}

PaintCommand::PaintCommand()
    : inUse(LFalse)
    , resource(kBoyiaNull)
{
}

PaintCommand::~PaintCommand()
{
}

LVoid PaintCommand::paint(Gdiplus::Graphics& gc)
{
    if (sPaintRegistrayArray) {
        (*sPaintRegistrayArray)[resource->type()]->paint(gc, this);
    }
}

PaintCommandAllocator::PaintCommandAllocator()
    : m_cmds(kBoyiaNull)
{
    initAllocator();
}

PaintCommandAllocator::~PaintCommandAllocator()
{
    if (m_cmds) {
        delete[] m_cmds;
    }
}

PaintCommandAllocator* PaintCommandAllocator::instance()
{
    static PaintCommandAllocator sAllocator;
    return &sAllocator;
}

LVoid PaintCommandAllocator::initAllocator()
{
    if (m_cmds) {
        delete[] m_cmds;
    }
    m_cmds = new PaintCommand[kPaintCommandDefaultCapacity];
    paintFunctionRegister();
}

PaintCommand* PaintCommandAllocator::alloc() const
{
    for (LInt i = 0; i < kPaintCommandDefaultCapacity; ++i) {
        if (!m_cmds[i].inUse) {
            m_cmds[i].inUse = LTrue;
            return &m_cmds[i];
        }
    }

    return kBoyiaNull;
}
}