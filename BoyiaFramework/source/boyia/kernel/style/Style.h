/*
 *  Style.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef Style_h
#define Style_h

#include "LGdi.h"

namespace util {

class Border {
public:
    Border();
    Border(const Border& border);
    ~Border();
    const Border& operator=(const Border& border);
    
    LVoid copy(const Border& border);
    LVoid init();

public:
    LInt topColor;
    LInt leftColor;
    LInt rightColor;
    LInt bottomColor;
    LInt topWidth;
    LInt leftWidth;
    LInt rightWidth;
    LInt bottomWidth;
    LInt topStyle;
    LInt leftStyle;
    LInt rightStyle;
    LInt bottomStyle;
};

class BorderRadius {
public:
    BorderRadius();
    BorderRadius(const BorderRadius& radius);
    
    LVoid copy(const BorderRadius& radius);
    
    LInt topLeftRadius;
    LInt topRightRadius;
    LInt bottomLeftRadius;
    LInt bottomRightRadius;
};

class Style {
public:
    // must be used with absolute or fixed
    typedef enum AlignType {
        ALIGN_NONE,
        ALIGN_TOP,
        ALIGN_LEFT,
        ALIGN_RIGHT,
        ALIGN_BOTTOM,
        ALIGN_CENTER,
        ALIGN_ALL
    } AlignType;

    typedef enum PositionType {
        ABSOLUTEPOSITION,
        FIXEDPOSITION,
        STATICPOSITION,
        RELATIVEPOSITION
    } PositionType;

    typedef enum DisplayType {
        DISPLAY_NONE,
        DISPLAY_BLOCK,
        DISPLAY_INLINE,
        DISPLAY_ANY
    } DisplayType;

    typedef enum FlexDirection {
        FLEX_NONE,
        FLEX_ROW,
        FLEX_COLUMN,
        FLEX_ROW_REVERSE,
        FLEX_COLUMN_REVERSE
    } FlexDirection;

public:
    Style();
    Style(LFont::FontStyle font);
    Style(const Style& style);
    const Style& operator=(const Style& style);
    void init();
    void copyStyle(Style* style);
    
    Border& border() const;
    BorderRadius& radius() const;

public:
    LColor color; // foreground color
    LColor bgColor; // background color
    String bgImageUrl;
    LFont font;
    LBool transparent; // if transparent
    LInt positionType;
    LInt displayType;
    LInt left;
    LInt top;
    LInt width;
    LInt height;
    LInt leftMargin;
    LInt rightMargin;
    LInt topMargin;
    LInt bottomMargin;
    LInt textAlignement;
    LInt topPadding;
    LInt bottomPadding;
    LInt leftPadding;
    LInt rightPadding;
    LReal scale;
    LInt zindex;
    LUint8 opacity;
    LUint8 drawOpacity;
    //Border border;
    
    LBool focusable;
    LInt flexDirection;
    LInt align;
    
private:
    mutable Border* m_border;
    mutable BorderRadius* m_radius;
};
}
#endif
