#include "LGdi.h"

namespace util {
class FontWin : public LFont {
public:
    FontWin(const LFont& font);
    virtual ~FontWin();

    virtual LInt getFontHeight() const;
    virtual LInt getFontWidth(LUint8 ch) const;
    virtual LInt getTextWidth(const String& text) const;
    virtual LInt calcTextLine(const String& text, LInt maxWidth) const;
    virtual LInt getLineSize() const;
    virtual LInt getLineWidth(LInt index) const;
    virtual LVoid getLineText(LInt index, String& text);
};

FontWin::FontWin(const LFont& font)
    : LFont(font)
{
}

FontWin::~FontWin()
{
}

LInt FontWin::getFontHeight() const
{
    return 0;
}

LInt FontWin::getFontWidth(LUint8 ch) const
{
    return 0;
}

LInt FontWin::getTextWidth(const String& text) const
{
    return 0;
}

LInt FontWin::getLineSize() const
{
    return 0;
}

LInt FontWin::getLineWidth(LInt index) const
{
    return 0;
}

LVoid FontWin::getLineText(LInt index, String& text)
{
}

LInt FontWin::calcTextLine(const String& text, LInt maxWidth) const
{
    return 0;
}

LFont* LFont::create(const LFont& font)
{
    return new FontWin(font);
}
}