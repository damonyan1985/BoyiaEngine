#ifndef FontWin_h
#define FontWin_h

#include "LGdi.h"
#include "KVector.h"
#include "OwnerPtr.h"

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

} // namespace util

#endif
