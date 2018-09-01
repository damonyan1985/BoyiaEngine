#include "JSImageView.h"
#include "ImageView.h"
#include "UIOperation.h"

namespace boyia
{
JSImageView::JSImageView(yanbo::HtmlView* item)
{
	m_item = item;
}

void JSImageView::loadImage(const String& url)
{
    //static_cast<yanbo::ImageView*>(m_item)->loadImage(url);
	yanbo::UIOperation::instance()->opLoadImageUrl(m_item, url);
}

void JSImageView::setImageUrl(const String& url)
{
	//static_cast<yanbo::ImageView*>(m_item)->setUrl(url);
	yanbo::UIOperation::instance()->opSetImageUrl(m_item, url);
}
}
