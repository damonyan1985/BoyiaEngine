/*
 *  Style.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef Style_h
#define Style_h

#include "LGdi.h"
#include "OwnerPtr.h"

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

class Margin {
public:
    Margin();
    LVoid copy(const Margin& margin);
    
    LInt leftMargin;
    LInt rightMargin;
    LInt topMargin;
    LInt bottomMargin;
};

class Padding {
public:
    Padding();
    LVoid copy(const Padding& padding);
    
    LInt topPadding;
    LInt bottomPadding;
    LInt leftPadding;
    LInt rightPadding;
};

class Flex {
public:
    Flex();
    LVoid copy(const Flex& flex);
    
    LInt flexGrow;
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
        FLEX_ROW_REVERSE,
        FLEX_ROW_SPACE_BETWEEN,
        FLEX_COLUMN,
        FLEX_COLUMN_REVERSE,
        FLEX_COLUMN_SPACE_BETWEEN
    } FlexDirection;

public:
    Style();
    Style(LFont::FontStyle font);
    Style(const Style& style);
    const Style& operator=(const Style& style);
    void init();
    
    Border& border() const;
    BorderRadius& radius() const;
    
    Margin& margin() const;
    Padding& padding() const;
    
    Flex& flex() const;
    LBool hasRadius() const;
    
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
    LInt textAlignement;
    LReal scale;
    LInt zindex;
    LUint8 opacity;
    LUint8 drawOpacity;
    
    LBool focusable;
    LInt flexDirection;
    LInt align;
    
private:
    // TextView等元素不需要border，border-radius
    // 因此这些样式采取动态创建的方式进行，减少内存开销
    mutable OwnerPtr<Border> m_border;
    mutable OwnerPtr<BorderRadius> m_radius;
    mutable OwnerPtr<Margin> m_margin;
    mutable OwnerPtr<Padding> m_padding;
    mutable OwnerPtr<Flex> m_flex;
};
}
#endif
