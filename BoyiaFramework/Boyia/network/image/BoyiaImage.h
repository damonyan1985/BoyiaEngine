#ifndef BoyiaImage_h
#define BoyiaImage_h

#include "HtmlView.h"
#include "ImageLoader.h"
#include "LGdi.h"
#include <stddef.h> // size_t

// 如果要开发操作系统或者无依赖框架
// 可以作为替换ImageAndroid和ImageIOS
// 的首选
namespace yanbo {

class BoyiaImage : public LImage, public ImageClient {
public:
    BoyiaImage();
    ~BoyiaImage();

    virtual LVoid setLoaded(LBool loaded);
    virtual LVoid setImageInfo(const ImageInfo& info);
    virtual LVoid load(const String& path, LVoid* image);
    virtual LVoid* item() const;
    virtual LVoid* pixels() const;
    LVoid setItem(HtmlView* item);
    virtual const String& url() const;

private:
    HtmlView* m_image;
    LByte* m_pixels;
};
}

#endif
