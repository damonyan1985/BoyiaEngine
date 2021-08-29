#include "ImageIOS.h"
#include "AppManager.h"
#include "ImageView.h"
#include "LColorUtil.h"
#include "LGdi.h"
#include "SalLog.h"
#include "ImageLoaderIOS.h"

namespace util {

ImageIOS::ImageIOS()
    : m_image(kBoyiaNull)
    , m_data(kBoyiaNull)
    , m_uimage(kBoyiaNull)
{
}

ImageIOS::~ImageIOS()
{
}

LVoid ImageIOS::load(const String& path, LVoid* image)
{
    yanbo::UIThreadClientMap::instance()->registerClient(this);
    NSString* nsUrl = [[NSString alloc] initWithUTF8String:GET_STR(path)];
    [[ImageLoaderIOS shareInstance] loadImage:nsUrl clientId:getClientId()];
}

LImage* LImage::create(LVoid* item)
{
    ImageIOS* image = new ImageIOS();
    image->setItem(static_cast<yanbo::HtmlView*>(item));
    return image;
}

void ImageIOS::drawText(const String& text,
    const LRect& rect,
    LGraphicsContext::TextAlign align,
    const LFont& font,
    const LColor& penColor,
    const LColor& brushColor)
{
}

// setLoaded在UIthread中执行
LVoid ImageIOS::setLoaded(LBool loaded)
{
    LImage::setLoaded(loaded);
    if (m_image && loaded) {
        yanbo::AppManager::instance()->uiThread()->drawUI(m_image);
    }
}

LVoid* ImageIOS::item() const
{
    return m_image;
}

LVoid ImageIOS::setItem(yanbo::HtmlView* item)
{
    m_image = item;
}

LVoid ImageIOS::setData(const OwnerPtr<String>& data)
{
    // 将数据转换为UIImage
    NSData* buffer = [[NSData alloc] initWithBytesNoCopy:data->GetBuffer() length:data->GetLength()];
    // 使用非拷贝方式转移data内部buffer的控制权
    data->ReleaseBuffer();
    m_uimage = [UIImage imageWithData:buffer];
}

const String& ImageIOS::url() const
{
    return static_cast<yanbo::ImageView*>(m_image)->url();
}

LVoid* ImageIOS::pixels() const
{
    // 从ARC指针转换为普通指针，释放交由开发者自己释放
    return (__bridge_retained void*)m_uimage;
}

LVoid ImageIOS::onClientCallback()
{
    setLoaded(LTrue);
}

}
