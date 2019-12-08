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

        float width = cmd->rect.GetWidth() * yanbo::PixelRatio::ratio();
        width = width < 1 ? 1 : width;
        float height = cmd->rect.GetHeight() * yanbo::PixelRatio::ratio();
        height = height < 1 ? 1 : height;
        Gdiplus::RectF rect(
            cmd->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
            cmd->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
            width,
            height
        );

        Gdiplus::SolidBrush brush(Gdiplus::Color(
            cmd->color.m_alpha,
            cmd->color.m_red,
            cmd->color.m_green,
            cmd->color.m_blue
        ));
        gc.FillRectangle(&brush, rect);
    }
};

class PaintTextHandler : public PaintCmdHandler {
public:
    virtual LVoid paint(Gdiplus::Graphics& gc, PaintCommand* cmd)
    {
        wstring wtext = yanbo::CharConvertor::CharToWchar(GET_STR(cmd->text));
        Gdiplus::Font font(L"Arial", cmd->font.getFontSize() * yanbo::PixelRatio::ratio());

        Gdiplus::StringFormat format(Gdiplus::StringAlignmentNear);
        Gdiplus::RectF rect(
            cmd->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
            cmd->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
            cmd->rect.GetWidth() * yanbo::PixelRatio::ratio(),
            cmd->rect.GetHeight() * yanbo::PixelRatio::ratio()
        );
        
        Gdiplus::SolidBrush brush(Gdiplus::Color(
            cmd->color.m_alpha,
            cmd->color.m_red,
            cmd->color.m_green,
            cmd->color.m_blue
        ));

        gc.DrawString(wtext.c_str(), wtext.length(), &font, rect, &format, &brush);
    }
};

class PaintImageHandler : public PaintCmdHandler {
public:
    virtual LVoid paint(Gdiplus::Graphics& gc, PaintCommand* cmd)
    {
        Gdiplus::Rect rect(
            cmd->rect.iTopLeft.iX * yanbo::PixelRatio::ratio(),
            cmd->rect.iTopLeft.iY * yanbo::PixelRatio::ratio(),
            cmd->rect.GetWidth() * yanbo::PixelRatio::ratio(),
            cmd->rect.GetHeight() * yanbo::PixelRatio::ratio()
        );
        gc.DrawImage(cmd->image, rect);
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
    , image(kBoyiaNull)
{
}

PaintCommand::~PaintCommand()
{
}

LVoid PaintCommand::paint(Gdiplus::Graphics& gc)
{
    if (sPaintRegistrayArray) {
        (*sPaintRegistrayArray)[type]->paint(gc, this);
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