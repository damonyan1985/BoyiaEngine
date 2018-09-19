/*
 * ImageView.cpp
 *
 *  Created on: 2011-11-14
 *      Author: Administrator
 */

#include "ImageView.h"
#include "LColor.h"
#include "StringUtils.h"
#include "ResourceLoader.h"
#include "RenderContext.h"
#include "SalLog.h"

namespace yanbo
{

ImageView::ImageView(
		const String& id,
		LBool selectable,
		const String& src)
    : InlineView(id, LFalse)
    , m_image(NULL)  // img item can't be selected
    , m_src(src)
{
}

ImageView::~ImageView()
{
	if (m_image)
	{
	    delete m_image;
	    m_image = NULL;
	}
}

void ImageView::paint(LGraphicsContext& gc)
{
	if (m_image && m_image->isLoaded())
	{
		gc.setHtmlView(this);
		setClipRect(gc);
		LayoutPoint topLeft = getAbsoluteContainerTopLeft();
		int x = topLeft.iX + getXpos();
		int y = topLeft.iY + getYpos();
		KFORMATLOG("ImageView::paint topLeft.iX=%d", topLeft.iY);
		KFORMATLOG("ImageView::paint getXpos()=%d", getXpos());
		KFORMATLOG("ImageView::paint x=%d", x);

		if (getStyle()->bgColor.m_alpha != 0)
		{
			KLOG("ImageView::paint drawBitmap begin");
			m_image->setRect(LRect(x, y, m_width, m_height));

		    LRgb bgColor = getStyle()->bgColor;
		    bgColor.m_alpha = bgColor.m_alpha * ((float)getStyle()->drawOpacity/ 255.0f);

			gc.setBrushColor(bgColor);
			gc.drawImage(m_image);
			KLOG("ImageView::paint drawBitmap end");
		}
	}
}

void ImageView::layoutInline(RenderContext& rc)
{
	KLOG("ImageView::layout begin");
	m_width = getStyle()->width > 0 ?
				(getStyle()->width * getStyle()->scale) : getParent()->getWidth();
	m_height = getStyle()->height > 0 ?
				(getStyle()->height * getStyle()->scale) : getParent()->getHeight();

	//rc.addLineItem(this);
	KFORMATLOG("ImageView::layout handleBefore getXpos()=%d rc.X=%d", getXpos(), rc.getX());
	handleXYPos(rc);

	KFORMATLOG("ImageView::layout getXpos()=%d rc.X=%d", getXpos(), getStyle()->left);
	if (m_x + m_width > rc.getMaxWidth())
	{
		rc.addY(m_height);
		rc.newLine(this);
	}

	if (getParent()->isBlockView())
	{
		rc.addLineItem(this);
		rc.addX(m_width);
		rc.setNextLineHeight(m_height);
	}
	else
	{
		m_x = m_x - getParent()->getXpos();
		m_y = m_y - getParent()->getYpos();
	}

	KFORMATLOG("Image width=%d", m_width);
	KFORMATLOG("Image height=%d", m_height);

	if (m_src.GetLength())
	{
		loadImage(m_src);
	}
}

void ImageView::setUrl(const String& url)
{
	m_src = url;
	if (!m_image)
	{
	    m_image = LImage::create(this);
	}

	m_image->setLoaded(LFalse);
}

void ImageView::loadImage(const String& url)
{
    if (!m_image)
    {
    	m_image = LImage::create(this);
    }

    if (!m_image->isLoaded() || !m_src.CompareCase(url))
    {
    	m_src = url;

        KFORMATLOG("ImageView::loadImage path=%s", (const char*)m_src.GetBuffer());
        m_image->load(url, this);
    }
}

LBool ImageView::isImage() const
{
    return LTrue;	
}

const String& ImageView::url() const
{
	return m_src;
}

}
