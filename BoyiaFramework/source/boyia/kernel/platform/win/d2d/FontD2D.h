#ifndef FontD2D_h
#define FontD2D_h

#include "../../../gui/LGdi.h"
#include "../../../util/KVector.h"
#include "../../../util/OwnerPtr.h"
#include <dwrite.h>

#pragma comment(lib, "dwrite.lib")

namespace util {

struct LineTextD2D;
class FontD2D : public LFont {
public:
    FontD2D(const LFont& font);
    virtual ~FontD2D();

    virtual LInt getFontHeight() const;
    virtual LInt getFontWidth(LUint16 ch) const;
    virtual LInt getTextWidth(const String& text) const;
    virtual LInt calcTextLine(const String& text, LInt maxWidth) const;
    virtual LInt getLineSize() const;
    virtual LInt getLineWidth(LInt index) const;
    virtual LVoid getLineText(LInt index, String& text);

private:
    IDWriteTextFormat* createTextFormat(float fontSize) const;

    mutable KVector<OwnerPtr<LineTextD2D>> m_lines;
    mutable LInt m_height;
};

} // namespace util

#endif
