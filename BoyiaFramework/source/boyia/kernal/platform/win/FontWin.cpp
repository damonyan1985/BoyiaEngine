#include "LGdi.h"
#include "AppManager.h"
#include "GraphicsContextWin.h"
#include "CharConvertor.h"
#include <Windows.h>
#include <GdiPlus.h>

using namespace Gdiplus;
typedef util::LString<wchar_t> WString;

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
    GraphicsPath path;
    FontFamily family;
    Font font(L"Arial", m_size);
    StringFormat format(Gdiplus::StringAlignmentNear);
    path.AddString(L"F", -1, &family, font.GetStyle(), font.GetSize(), Gdiplus::Point(0, 0), &format);
    
    Gdiplus::Rect rect;
    path.GetBounds(&rect);
    return rect.Height;
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
    GraphicsPath path;
    FontFamily family;
    Font font(L"Arial", m_size);
    StringFormat format(Gdiplus::StringAlignmentNear);
    Gdiplus::Rect rect;

    LInt currentLineWidth = 0;
    LInt maxLineWidth = 0;

    wstring wtext = yanbo::CharConvertor::CharToWchar(GET_STR(text));
    WString wstr((wchar_t)0, 100);
    for (LInt i = 0; i < wtext.length(); ++i) {
        wchar_t ch = wtext.at(i);
        path.AddString(&ch, 1, &family, font.GetStyle(), font.GetSize(), Gdiplus::Point(0, 0), &format);
        path.GetBounds(&rect);
        
        if (currentLineWidth + rect.Width <= maxWidth) {
            wstr += ch;
            currentLineWidth += rect.Width;
        } else {
            maxLineWidth = maxLineWidth < currentLineWidth ?
                currentLineWidth : maxLineWidth;
            OwnerPtr<String> lineText = new String();
            yanbo::CharConvertor::WcharToChar(wstr.GetBuffer(), *lineText.get());
            m_lines.addElement(new LineText(lineText, currentLineWidth));
            currentLineWidth = 0;
            wstr.ClearBuffer();
        }
    }

    if (currentLineWidth > 0) {
        maxLineWidth = maxLineWidth < currentLineWidth ?
            currentLineWidth : maxLineWidth;

        OwnerPtr<String> lineText = new String();
        yanbo::CharConvertor::WcharToChar(wstr.GetBuffer(), *lineText.get());
        m_lines.addElement(new LineText(lineText, currentLineWidth));
    }

    return maxLineWidth;
}

LFont* LFont::create(const LFont& font)
{
    return new FontWin(font);
}
}