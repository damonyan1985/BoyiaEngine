/*
 * LBoxItem.h
 *
 *  Created on: 2011-6-22
 *      Author: yanbo
 */

#ifndef LBaseView_h
#define LBaseView_h

#include "Style.h"
#include "BoyiaRef.h"

namespace util {

/**
 * LBaseView. 所有View的基类
 * 隶属于GuiLayer即抽象接口层
 */
// 目前不能使HtmlView继承自ViewPainter，HtmlView指针转换成void*或者intptr后
// 使用ViewPainter*强转将无法正常引用ViewPainter中的内存，
// 必须再使用HtmlView*将void*或者intptr强转后，才能使用ViewPainter*引用，
// 可以参看C++多重继承中内存模型原理
class LBaseView : public ViewPainter {
public:
    LBaseView();
    virtual ~LBaseView();

public:
    LVoid setTransparent(LBool transparent);
    LBool getTransparency();

    LVoid setFont(LFont font);
    const LFont& getFont() const;

    LVoid setPos(LInt x, LInt y);
    LVoid setSize(LInt width, LInt height);

    LVoid translate(LInt x, LInt y);

    LVoid setXpos(LInt x);
    LVoid setYpos(LInt y);

    LInt getXpos() const;
    LInt getYpos() const;

    virtual LInt getWidth() const;
    virtual LInt getHeight() const;

    LVoid setWidth(LInt width);
    LVoid setHeight(LInt height);

    LVoid setPadding(LInt leftPadding, LInt topPadding);

    LVoid setVisible(LBool visible);

    virtual LInt getEndX();
    virtual LInt getBottomY();

    LVoid setTagName(const String& tagName);
    const String& getTagName() const;

protected:
    LBool m_visible;

    /** The x position of this item */
    LInt m_x;

    /** The y position of this item */
    LInt m_y;

    /** The width in pixels of this item */
    LInt m_width;

    /** The height in pixels of this item */
    LInt m_height;

    /** The number of padding pixels from the leftmost border of this item */
    LInt m_leftPadding;

    /** The number of padding pixels from the topmost border of this item */
    LInt m_topPadding;
    String m_tagName;
    Style m_style;
};
}

using util::LBaseView;
#endif /* LGRAPHICITEM_H_ */
