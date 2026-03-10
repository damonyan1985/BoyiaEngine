#include "FontD2D.h"
#include "CharConvertor.h"
#include "PixelRatio.h"
#include "UtilString.h"
#include <dwrite.h>

namespace util {

static IDWriteFactory* getDWriteFactory()
{
    static IDWriteFactory* s_factory = NULL;
    if (!s_factory) {
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&s_factory);
    }
    return s_factory;
}

static DWRITE_FONT_WEIGHT toWeight(LFont::FontStyle style)
{
    return (style == LFont::FONT_STYLE_BOLD) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL;
}

static DWRITE_FONT_STYLE toStyle(LFont::FontStyle style)
{
    return (style == LFont::FONT_STYLE_ITALIC) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL;
}

FontD2D::FontD2D(const LFont& font)
    : LFont(font)
    , m_height(0)
{
}

FontD2D::~FontD2D()
{
}

IDWriteTextFormat* FontD2D::createTextFormat(float fontSize) const
{
    IDWriteFactory* factory = getDWriteFactory();
    if (!factory) {
        return NULL;
    }
    WString wFamily;
    yanbo::CharConvertor::CharToWchar(getFamily(), wFamily);
    const wchar_t* family = wFamily.GetLength() > 0 ? wFamily.GetBuffer() : L"Arial";
    if (fontSize < 1.0f) {
        fontSize = 12.0f;
    }
    IDWriteTextFormat* format = NULL;
    HRESULT hr = factory->CreateTextFormat(
        family,
        NULL,
        toWeight(getFontStyle()),
        toStyle(getFontStyle()),
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"",
        &format);
    return SUCCEEDED(hr) ? format : NULL;
}

LInt FontD2D::getFontHeight() const
{
    return m_height;
}

LInt FontD2D::getFontWidth(LUint16 ch) const
{
    wchar_t wbuf[2] = { (wchar_t)ch, 0 };
    WString wstr(wbuf, 1, LTrue);
    String s;
    yanbo::CharConvertor::WcharToChar(wstr, s);
    return getTextWidth(s);
}

LInt FontD2D::getTextWidth(const String& text) const
{
    if (text.GetLength() == 0) {
        return 0;
    }
    WString wtext;
    yanbo::CharConvertor::CharToWchar(text, wtext);
    if (wtext.GetLength() == 0) {
        return 0;
    }
    IDWriteTextFormat* format = createTextFormat(getFontSize() * yanbo::PixelRatio::ratio());
    if (!format) {
        return 0;
    }
    IDWriteFactory* factory = getDWriteFactory();
    if (!factory) {
        format->Release();
        return 0;
    }
    IDWriteTextLayout* layout = NULL;
    HRESULT hr = factory->CreateTextLayout(
        wtext.GetBuffer(),
        (UINT32)wtext.GetLength(),
        format,
        10000.0f,
        10000.0f,
        &layout);
    format->Release();
    if (FAILED(hr) || !layout) {
        return 0;
    }
    DWRITE_TEXT_METRICS metrics;
    layout->GetMetrics(&metrics);
    layout->Release();
    LInt width = (LInt)(metrics.widthIncludingTrailingWhitespace + 0.5f);
    return yanbo::PixelRatio::viewX(width);
}

LInt FontD2D::getLineSize() const
{
    return (LInt)m_lines.size();
}

LInt FontD2D::getLineWidth(LInt index) const
{
    if (index < 0 || index >= (LInt)m_lines.size()) {
        return 0;
    }
    return m_lines.elementAt(index)->width;
}

LVoid FontD2D::getLineText(LInt index, String& text)
{
    if (index >= 0 && index < (LInt)m_lines.size()) {
        text = *m_lines.elementAt(index)->text.get();
    }
}

LInt FontD2D::calcTextLine(const String& text, LInt maxWidth) const
{
    m_lines.clear();
    m_height = 0;

    if (text.GetLength() == 0) {
        return 0;
    }

    LInt fontSize = (LInt)(getFontSize() * yanbo::PixelRatio::ratio());
    if (fontSize < 1) {
        fontSize = 12;
    }
    IDWriteTextFormat* format = createTextFormat((float)fontSize);
    if (!format) {
        return 0;
    }

    IDWriteFactory* factory = getDWriteFactory();
    if (!factory) {
        format->Release();
        return 0;
    }

    maxWidth = yanbo::PixelRatio::rawX(maxWidth);

    WString wtext;
    yanbo::CharConvertor::CharToWchar(text, wtext);
    const wchar_t* buffer = wtext.GetBuffer();
    LInt count = 1;
    LInt currentLineWidth = 0;
    LInt maxLineWidth = 0;
    LInt height = 0;

    while (*(buffer + count - 1)) {
        IDWriteTextLayout* layout = NULL;
        HRESULT hr = factory->CreateTextLayout(
            buffer,
            (UINT32)count,
            format,
            10000.0f,
            10000.0f,
            &layout);
        if (FAILED(hr) || !layout) {
            format->Release();
            return 0;
        }

        DWRITE_TEXT_METRICS metrics;
        layout->GetMetrics(&metrics);
        layout->Release();

        LInt stringWidth = (LInt)(metrics.widthIncludingTrailingWhitespace + 0.5f) + 1;
        LInt stringHeight = (LInt)(metrics.height + 0.5f) + 1;

        if (stringWidth <= maxWidth) {
            currentLineWidth = stringWidth;
            count++;
        } else {
            maxLineWidth = maxLineWidth < currentLineWidth ? currentLineWidth : maxLineWidth;
            count -= 1;
            if (count > 0) {
                WString wstr(buffer, count, LTrue);
                OwnerPtr<String> lineText = new String();
                yanbo::CharConvertor::WcharToChar(wstr, *lineText.get());
                m_lines.addElement(new LineTextD2D{ lineText, yanbo::PixelRatio::viewX(currentLineWidth) });
            }
            currentLineWidth = 0;
            buffer += (count > 0 ? count : 1);
            count = 1;
        }

        height = height < stringHeight ? stringHeight : height;
    }

    if (currentLineWidth > 0) {
        maxLineWidth = maxLineWidth < currentLineWidth ? currentLineWidth : maxLineWidth;
        WString wstr(buffer, count - 1, LTrue);
        OwnerPtr<String> lineText = new String();
        yanbo::CharConvertor::WcharToChar(wstr, *lineText.get());
        m_lines.addElement(new LineTextD2D{ lineText, yanbo::PixelRatio::viewX(currentLineWidth) });
    }

    format->Release();

    if (height > 0) {
        m_height = yanbo::PixelRatio::viewY(height);
    }

    return yanbo::PixelRatio::viewX(maxLineWidth);
}

} // namespace util
