#ifndef FontAndroid_h
#define FontAndroid_h

#include "AutoObject.h"
#include "UtilString.h"

namespace util {

struct JFontAndroid;
class FontAndroid {
public:
    FontAndroid();
    virtual ~FontAndroid();
    void init(JNIEnv* env, jclass clazz, jobject o);
    LInt getFontHeight(LInt size) const;
    LInt getFontWidth(LUint8 ch, LInt size) const;
    LInt getTextWidth(const String& text, LInt size) const;
    LInt getLineSize() const;
    LInt getLineWidth(LInt index) const;
    LVoid getLineText(LInt index, String& text);
    LInt calcTextLine(const String& text, LInt maxWidth, LInt fontSize) const;
    LInt getIndexByOffset(LInt line, LInt x) const;
    LInt getOffsetByIndex(LInt line, LInt index) const;

private:
    struct JFontAndroid* m_privateFont;
};
}

#endif
