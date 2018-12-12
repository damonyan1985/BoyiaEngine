/*
 * LBoxItem.h
 *
 *  Created on: 2011-6-22
 *      Author: yanbo
 */

#ifndef LBaseView_h
#define LBaseView_h

#include "PlatformLib.h"
#include "Style.h"
#include "KRef.h"
#include "KRefPtr.h"
#include <stdlib.h>

namespace util
{

/**
 * LBaseView. 所有View的基类
 * 隶属于GuiLayer即抽象接口层
 */

class LBaseView
{
public:
	LBaseView();
	virtual ~LBaseView();
	
public:
	void setTransparent(LBool transparent);
	LBool getTransparency();
	
	void setFont(LFont font);
	const LFont& getFont() const;
	
	void setPos(LInt x, LInt y);
	void setSize(LInt width, LInt height);
	
	void translate(LInt x, LInt y);
	
	void setXpos(LInt x);
	void setYpos(LInt y);

	LInt getXpos() const;
	LInt getYpos() const;
	
	virtual LInt getWidth() const;
	virtual LInt getHeight() const;
	
	void setWidth(LInt width);
	void setHeight(LInt height);
	
	void setPadding(LInt leftPadding, LInt topPadding);
	
	void setVisible(LBool visible);
	
	virtual int getEndX();
	virtual int getBottomY();

	void setTagName(const String& tagName);
	const String& getTagName() const;

	void* operator new(size_t sz);
	void operator delete(void *p);

protected:
	LBool           m_visible;

	/** The x position of this item */
	LInt            m_x;

	/** The y position of this item */
	LInt            m_y;

	/** The width in pixels of this item */
	LInt            m_width;

	/** The height in pixels of this item */
	LInt            m_height;

	/** The number of padding pixels from the leftmost border of this item */
	LInt            m_leftPadding;

	/** The number of padding pixels from the topmost border of this item */
	LInt            m_topPadding;
	String          m_tagName;
	Style           m_style;
};
}
#endif /* LGRAPHICITEM_H_ */
