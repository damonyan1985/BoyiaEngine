#include "LGdi.h"
#include "AppManager.h"
#include "CharConvertor.h"
#include "PixelRatio.h"
#include <Windows.h>
#include <GdiPlus.h>

using namespace Gdiplus;

namespace util {
class LineText {
public:
    LineText(OwnerPtr<String> ptr, LInt tw)
        : width(tw)
        , text(ptr)
    {
    }

    OwnerPtr<String> text;
    LInt width;
};

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

private:
    mutable KVector<OwnerPtr<LineText>> m_lines;
    mutable LInt m_height;
};

FontWin::FontWin(const LFont& font)
    : LFont(font)
    , m_height(0)
{
}

FontWin::~FontWin()
{
}

LInt FontWin::getFontHeight() const
{
    return m_height;
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
    return m_lines.size();
}

LInt FontWin::getLineWidth(LInt index) const
{
    return m_lines.elementAt(index)->width;
}

LVoid FontWin::getLineText(LInt index, String& text)
{
    text = *m_lines.elementAt(index)->text.get();
}

LInt FontWin::calcTextLine(const String& text, LInt maxWidth) const
{
    LInt fontSize = getFontSize() * yanbo::PixelRatio::ratio();

    FontFamily family;
    Font font(L"Arial", fontSize,
        FontStyleRegular, UnitPixel);
    font.GetFamily(&family);

    maxWidth = yanbo::PixelRatio::rawX(maxWidth);

    StringFormat format(Gdiplus::StringAlignmentNear);
    Gdiplus::Rect rect;

    LInt currentLineWidth = 0;
    LInt maxLineWidth = 0;

    wstring wtext = yanbo::CharConvertor::CharToWchar(GET_STR(text));
    const wchar_t* buffer = wtext.c_str();
    LInt count = 1;
    LInt height = 0;
    while (*(buffer + count - 1)) {
        Gdiplus::GraphicsPath path;
        path.AddString(buffer, count, &family, font.GetStyle(), font.GetSize(), Gdiplus::Point(0, 0), &format);
        path.GetBounds(&rect);

        if (rect.Width <= maxWidth) {
            currentLineWidth = rect.Width;
            count++;
        } else {
            maxLineWidth = maxLineWidth < currentLineWidth ?
                currentLineWidth : maxLineWidth;
            count -= 1;
            WString wstr(buffer, count, LTrue);
            OwnerPtr<String> lineText = new String();
            yanbo::CharConvertor::WcharToChar(wstr, *lineText.get());
            m_lines.addElement(new LineText(lineText, yanbo::PixelRatio::viewX(currentLineWidth + 8)));
            currentLineWidth = 0;

            buffer += count;
            count = 1;
        }

        height = height < rect.Height ? rect.Height : height;
    }

    if (currentLineWidth > 0) {
        maxLineWidth = maxLineWidth < currentLineWidth ?
            currentLineWidth : maxLineWidth;

        OwnerPtr<String> lineText = new String();
        WString wstr(buffer, count, LTrue);
        yanbo::CharConvertor::WcharToChar(wstr, *lineText.get());
        m_lines.addElement(new LineText(lineText, yanbo::PixelRatio::viewX(currentLineWidth + 8)));
    }

    if (height) {
        m_height = yanbo::PixelRatio::viewY(height + 4);
    }

    return yanbo::PixelRatio::viewX(maxLineWidth);
}

LFont* LFont::create(const LFont& font)
{
    return new FontWin(font);
}
}