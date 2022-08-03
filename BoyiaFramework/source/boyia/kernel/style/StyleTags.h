/*
 * StyleTags.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef StyleTags_h
#define StyleTags_h

//#include "kmap.h"
//#include "IdentityMap.h"
//#include "UtilString.h"
#include "HashMap.h"
#include "HashUtil.h"

namespace util {
/**
 * Objects of this class holds the symbol table for CSS tags.
 * See this class for more info.
 */
class StyleTags {
public:
    //properties
    //properties that are inherited has value > STYLE_NULL.
    //properties that is not inherited has value < STYLE_NULL.
    enum StyleType {
        TAGNONE = 0,
        ALIGN,
        WIDTH,
        HEIGHT,
        DISPLAY,
        FLEX_DIRECTION,
        FLEX_GROW,
        FLOAT,
        BACKGROUND_IMAGE,
        PADDING,
        PADDING_TOP,
        PADDING_LEFT,
        PADDING_BOTTOM,
        PADDING_RIGHT,
        BORDER_TOP,
        BORDER_TOP_WIDTH,
        BORDER_BOTTOM,
        BORDER_BOTTOM_WIDTH,
        BORDER_RADIUS,
        BORDER_TOP_LEFT_RADIUS,
        BORDER_TOP_RIGHT_RADIUS,
        BORDER_BOTTOM_LEFT_RADIUS,
        BORDER_BOTTOM_RIGHT_RADIUS,
        MARGIN,
        MARGIN_RIGHT,
        MARGIB_BOTTOM,
        MARGIN_TOP,
        MARGIN_LEFT,
        BORDER,
        BORDER_STYLE,
        BORDER_COLOR,
        BORDER_TOP_STYLE,
        BORDER_TOP_COLOR,
        BORDER_LEFT_STYLE,
        BORDER_LEFT_COLOR,
        BORDER_LEFT_WIDTH,
        BORDER_RIGHT_STYLE,
        BORDER_RIGHT_COLOR,
        BORDER_RIGHT_WIDTH,
        BORDER_BOTTOM_COLOR,
        BORDER_BOTTOM_STYLE,
        POSITION,
        LEFT,
        TOP,
        BACKGROUND,
        BACKGROUND_COLOR,
        Z_INDEX,
        FOCUSABLE,
        STYLE_INHERIT, // inherit之后的元素需要被继承
        COLOR,
        FONT,
        FONT_SIZE,
        FONT_STYLE,
        FONT_WEIGHT,
        FONT_FAMILY,
        TEXT_ALIGN,
        SCALE,
        TAGEND,
    };

public:
    static StyleTags* getInstance();
    static void destroyInstance();

    void defineInitialSymbol(const String& cssText, LInt cssType);
    ~StyleTags();

    LInt symbolAsInt(const String& key);

private:
    StyleTags();

private:
    static StyleTags* s_tags;
    //IdentityMap m_map;
    HashMap<HashString, LInt> m_map;
};
}
#endif
