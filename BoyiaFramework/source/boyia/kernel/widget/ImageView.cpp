/*
 * ImageView.cpp
 *
 *  Created on: 2011-11-14
 *      Author: yanbo
 */

#include "ImageView.h"
#include "LColorUtil.h"
#include "RenderContext.h"
#include "ResourceLoader.h"
#include "SalLog.h"
#include "StringUtils.h"

namespace yanbo {

ImageView::ImageView(
    const String& id,
    LBool selectable,
    const String& src)
    : InlineView(id, LFalse) // img item can't be selected
    , m_image(LImage::create(this))
    , m_src(src)
{
    if (m_src.GetLength()) {
        m_image->load(m_src, this);
    }
}

ImageView::~ImageView()
{
}

LVoid ImageView::paint(LGraphicsContext& gc)
{
    if (m_image && m_image->isLoaded()) {
        gc.setHtmlView(this);
        setClipRect(gc);

        LayoutPoint topLeft = getAbsoluteContainerTopLeft();
        LInt x = topLeft.iX + getXpos();
        LInt y = topLeft.iY + getYpos();
        BOYIA_LOG("ImageView::paint topLeft.iX=%d", topLeft.iY);
        BOYIA_LOG("ImageView::paint getXpos()=%d", getXpos());
        BOYIA_LOG("ImageView::paint x=%d", x);

        if (getStyle()->bgColor.m_alpha != 0) {
            KLOG("ImageView::paint drawBitmap begin");
            m_image->setRect(LRect(x, y, m_width, m_height));

            LColor bgColor = getStyle()->bgColor;
            bgColor.m_alpha = bgColor.m_alpha * ((float)getStyle()->drawOpacity / 255.0f);

            gc.setBrushColor(bgColor);
            //gc.drawImage(m_image);
            if (getStyle()->hasRadius()) {
                gc.drawRoundImage(m_image,
                                 getStyle()->radius().topLeftRadius,
                                 getStyle()->radius().topRightRadius,
                                 getStyle()->radius().bottomRightRadius,
                                 getStyle()->radius().bottomLeftRadius);
            } else {
                gc.drawImage(m_image);
            }
            KLOG("ImageView::paint drawBitmap end");
        }

        gc.restore();
    }
}

LVoid ImageView::layoutInline(RenderContext& rc)
{
    KLOG("ImageView::layout begin");
    m_width = getStyle()->width > 0 ? getScaleWidth() : getParent()->getWidth();
    m_height = getStyle()->height > 0 ? getScaleHeight() : getParent()->getHeight();

    //rc.addLineItem(this);
    BOYIA_LOG("ImageView::layout handleBefore getXpos()=%d rc.X=%d", getXpos(), rc.getX());
    handleXYPos(rc);

    BOYIA_LOG("ImageView::layout getXpos()=%d rc.X=%d", getXpos(), getStyle()->left);
    if (m_x + m_width > rc.getMaxWidth()) {
        rc.addY(m_height);
        rc.newLine(this);
    }

    if (getParent()->isBlockView()) {
        rc.addLineItem(this);
        rc.addX(m_width);
        rc.setNextLineHeight(m_height);
    } else {
        m_x = m_x - getParent()->getXpos();
        m_y = m_y - getParent()->getYpos();
    }

    BOYIA_LOG("Image width=%d", m_width);
    BOYIA_LOG("Image height=%d", m_height);
}

LVoid ImageView::setUrl(const String& url)
{
    if (m_src.CompareCase(url)) {
        return;
    }
    m_src = url;
    m_image->setLoaded(LFalse);
    m_image->load(url, this);
    BOYIA_LOG("ImageView::setUrl path=%s", GET_STR(m_src));
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
