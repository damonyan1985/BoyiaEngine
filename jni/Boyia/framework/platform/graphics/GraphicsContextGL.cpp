#include "GraphicsContextGL.h"
#include "ShaderUtil.h"
#include "MatrixState.h"
#include "ImageAndroid.h"
#include "HtmlView.h"
#include "UIView.h"
#include "JNIUtil.h"
#include "GLContext.h"
#include "MiniTextureCache.h"
#include "MediaPlayerAndroid.h"
#include <stdlib.h>
#include <android/bitmap.h>
#include <GLES3/gl3.h>
//#include <GLES2/gl2ext.h>

namespace util
{

GraphicsContextGL::GraphicsContextGL()
    : m_item(NULL)
{
}

GraphicsContextGL::~GraphicsContextGL()
{
	//GLContext::destroyGLContext();
}

LVoid GraphicsContextGL::setHtmlView(LVoid* item)
{
    m_item = item;
    ItemPainter* painter = currentPainter();
    painter->painters.clear();
}

ItemPainter* GraphicsContextGL::currentPainter()
{
    yanbo::HtmlView* item = (yanbo::HtmlView*) m_item;
    ItemPainter* painter = (ItemPainter*) item->painter();
    if (!painter)
    {
        painter = new ItemPainter();
        painter->item = item;
        item->setPainter(painter);
    }

    return painter;
}

LVoid GraphicsContextGL::drawLine(const LPoint& p1, const LPoint& p2)
{
    ItemPainter* painter = currentPainter();
	KRefPtr<yanbo::GLPainter> line = new yanbo::GLPainter();
    line->setLine(p1, p2);
    line->setColor(m_penColor);
    painter->painters.push(line);
}

LVoid GraphicsContextGL::drawLine(LInt x0, LInt y0, LInt x1, LInt y1)
{
	drawLine(LPoint(x0, y0), LPoint(x1, y1));
}

LVoid GraphicsContextGL::drawRect(const LRect& aRect)
{
    ItemPainter* painter = currentPainter();
    KRefPtr<yanbo::GLPainter> shapeRect = new yanbo::GLPainter();
	shapeRect->setRect(aRect);
	//m_brushColor.m_alpha = 0x33;
	shapeRect->setColor(m_brushColor);
    painter->painters.push(shapeRect);
}

LVoid GraphicsContextGL::drawRect(LInt x, LInt y, LInt w, LInt h)
{
	drawRect(LRect(x, y, w, h));
}

LVoid GraphicsContextGL::drawHollowRect(const LRect& rect)
{

}

LVoid GraphicsContextGL::drawHollowRect(LInt x, LInt y, LInt w, LInt h)
{
	drawLine(x, y, x+w, y);
	drawLine(x, y, x, y+h);
	drawLine(x+w, y, x+w, y+h);
	drawLine(x, y+h, x+w, y+h);
}

LVoid GraphicsContextGL::drawEllipse(const LRect& aRect)
{

}

LVoid GraphicsContextGL::drawRoundRect(const LRect& aRect, const LSize& aCornerSize)
{

}

LVoid GraphicsContextGL::drawText(const String& aText, const LRect& aRect)
{

}

LVoid GraphicsContextGL::drawText(const String& aText, const LPoint& aPoint)
{

}

LVoid GraphicsContextGL::drawImage(const LPoint& aTopLeft, const LImage* aBitmap)
{

}

LVoid GraphicsContextGL::drawImage(const LImage* image)
{
    if (image == NULL)
    {
    	return;
    }

    yanbo::MiniTexture* tex = yanbo::MiniTextureCache::getInst()->put(image);

    ItemPainter* painter = currentPainter();
    KRefPtr<yanbo::GLPainter> paint = new yanbo::GLPainter();
    paint->setColor(m_brushColor);
    paint->setImage(tex, image->rect());
    painter->painters.push(paint);

    KLOG("GraphicsContextGL::drawBitmap end");
}

LVoid GraphicsContextGL::drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect)
{
}

LVoid GraphicsContextGL::drawVideo(const LRect& rect, const LMediaPlayer* mp)
{
    //KLOG("GraphicsContextGL::drawBitmap drawVideo");

	MediaPlayerAndroid* amp = (MediaPlayerAndroid*) mp;
	if (!amp->texId()) return;


	KRefPtr<yanbo::MiniTexture> tex = new yanbo::MiniTexture();
	tex->width = rect.GetWidth();
	tex->height = rect.GetHeight();
	tex->texId = amp->texId();

	//KFORMATLOG("GraphicsContextGL drawVideo error=%d", glGetError());

	ItemPainter* painter = currentPainter();
	KRefPtr<yanbo::GLPainter> paint = new yanbo::GLPainter();
	paint->setColor(LRgb(0,0,0,0xFF));

	paint->setVideo(tex.get(), rect);
	amp->updateTexture(paint->stMatrix());
	//KFORMATLOG("GraphicsContextGL drawVideo error=%d texId=%d", glGetError(), tex->texId);

	painter->painters.push(paint);
}

LVoid GraphicsContextGL::drawText(const String& text, const LRect& rect, TextAlign align)
{
	KRefPtr<ImageAndroid> image = new ImageAndroid();
	image->drawText(text, rect, align, m_font, m_penColor, m_brushColor);
	image->unlockPixels();
	image->setRect(rect);
	image->setItem((yanbo::HtmlView*)m_item);
	if (yanbo::MiniTextureCache::getInst()->find((yanbo::HtmlView*)m_item))
	{
		yanbo::MiniTexture* tex = yanbo::MiniTextureCache::getInst()->updateTexture(image.get());
		ItemPainter* painter = currentPainter();
		KRefPtr<yanbo::GLPainter> paint = new yanbo::GLPainter();
		paint->setColor(m_brushColor);
		paint->setImage(tex, image->rect());
		painter->painters.push(paint);
	}
	else
	{
		drawImage(image.get());
	}
}

LVoid GraphicsContextGL::setBrushStyle(BrushStyle aBrushStyle)
{

}

LVoid GraphicsContextGL::setPenStyle(PenStyle aPenStyle)
{

}

LVoid GraphicsContextGL::setBrushColor(const LRgb& aColor)
{
    m_brushColor = aColor;
}

LVoid GraphicsContextGL::setPenColor(const LRgb& aColor)
{
	m_penColor = aColor;
}

LVoid GraphicsContextGL::setFont(const LFont& font)
{
	m_font = font;
}

LVoid GraphicsContextGL::reset()
{
//    KList<ItemPainter*>::Iterator iter = m_painters.begin();
//    KList<ItemPainter*>::Iterator iterEnd = m_painters.end();
//    for (; iter != iterEnd; ++iter)
//    {
//        ItemPainter* painter = (*iter);
//        yanbo::HtmlView* item = (yanbo::HtmlView*)painter->item;
//        item->setPainter(NULL);
//        delete painter;
//    }
//
//	m_painters.clear();
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

LVoid GraphicsContextGL::submit(LVoid* ptr)
{
	yanbo::HtmlView* item = (yanbo::HtmlView*) ptr;
	ItemPainter* painter = (ItemPainter*)item->painter();
	if (!painter)
	{
		return;
	}

    if (item->isClipItem())
    {
        KLOG("GraphicsContextGL::submit clip");
        glEnable(GL_SCISSOR_TEST);
        LRect rect = item->clipRect();
        int x = 0;
        int y = 0;
        yanbo::ShaderUtil::screenToGlPixel(
                rect.iTopLeft.iX,
                rect.iBottomRight.iY,
                &x,&y
        );
        glScissor(
                x,
                y,
                rect.GetWidth(),
                rect.GetHeight()
        );
    }

    ListPainter::Iterator glIter = painter->painters.begin();
    ListPainter::Iterator glIterEnd = painter->painters.end();
    for (; glIter != glIterEnd; ++glIter)
    {
        (*glIter)->paint();
    }

    if (item->isClipItem())
    {
        glDisable(GL_SCISSOR_TEST);//禁用剪裁测试
    }

	yanbo::HtmlViewList::Iterator iter    = item->m_children.begin();
	yanbo::HtmlViewList::Iterator iterEnd = item->m_children.end();

	for (; iter != iterEnd; ++iter)
	{
        yanbo::HtmlView* childItem = *iter;
        submit(childItem);
	}
}

LVoid GraphicsContextGL::fillBuffer(LVoid* ptr)
{
	yanbo::HtmlView* item = (yanbo::HtmlView*) ptr;
	ItemPainter* painter = (ItemPainter*)item->painter();
	if (!painter)
	{
		return;
	}

	ListPainter::Iterator glIter = painter->painters.begin();
	ListPainter::Iterator glIterEnd = painter->painters.end();
	for (; glIter != glIterEnd; ++glIter)
	{
	    (*glIter)->appendToBuffer();
	}


    yanbo::HtmlViewList::Iterator iter    = item->m_children.begin();
	yanbo::HtmlViewList::Iterator iterEnd = item->m_children.end();

	for (; iter != iterEnd; ++iter)
	{
	    yanbo::HtmlView* childItem = *iter;
	    fillBuffer(childItem);
	}
}

LVoid GraphicsContextGL::submit()
{
	yanbo::HtmlView* item = yanbo::UIView::getInstance()->getDocument()->getRenderTreeRoot();
	fillBuffer(item);
	yanbo::GLPainter::bindVBO();

	//glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	submit(item);

	glDisable(GL_BLEND);
	//glDisable(GL_TEXTURE_2D);

	yanbo::GLPainter::unbindVBO();
	//GLContext::postToScreen();
}

}
