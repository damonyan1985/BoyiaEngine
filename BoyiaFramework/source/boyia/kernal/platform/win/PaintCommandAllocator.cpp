#include "PaintCommandAllocator.h"
#include "CharConvertor.h"
#include "KVector.h"

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
        Gdiplus::Rect rect(
            cmd->rect.iTopLeft.iX,
            cmd->rect.iTopLeft.iY,
            cmd->rect.GetWidth(),
            cmd->rect.GetHeight()
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
        Gdiplus::Font font(L"Arial", cmd->font.getFontSize());

        Gdiplus::StringFormat format(Gdiplus::StringAlignmentNear);
        Gdiplus::RectF rect(
            cmd->rect.iTopLeft.iX,
            cmd->rect.iTopLeft.iY,
            cmd->rect.GetWidth(),
            cmd->rect.GetHeight()
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
            cmd->rect.iTopLeft.iX,
            cmd->rect.iTopLeft.iY,
            cmd->rect.GetWidth(),
            cmd->rect.GetHeight()
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