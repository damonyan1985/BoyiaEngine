#include "BoyiaImageView.h"
#include "ImageView.h"
#include "UIOperation.h"

namespace boyia {
BoyiaImageView::BoyiaImageView(BoyiaRuntime* runtime, yanbo::HtmlView* item)
    : BoyiaView(runtime)
{
    m_item = item;
}

LVoid BoyiaImageView::loadImage(const String& url)
{
    //static_cast<yanbo::ImageView*>(m_item)->loadImage(url);
    runtime()->view()->operation()->opLoadImageUrl(m_item, url);
}

LVoid BoyiaImageView::setImageUrl(const String& url)
{
    //static_cast<yanbo::ImageView*>(m_item)->setUrl(url);
    runtime()->view()->operation()->opSetImageUrl(m_item, url);
}
}
