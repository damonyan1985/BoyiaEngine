#ifndef NetImage_h
#define NetImage_h

#include "HtmlView.h"
#include <stddef.h> // size_t

// 如果要开发操作系统或者无依赖框架
// 可以作为替换ImageAndroid和ImageIOS
// 的首选
namespace yanbo
{
class NetImage : public LImage
{
public:
	enum ImageType
	{
		ENETIMAGE_NONE,
		ENETIMAGE_JPEG,
		ENETIMAGE_PNG,
		ENETIMAGE_GIF
	};
	NetImage();

    LInt getType(const char* data);
    LVoid readJPEG(const LByte* data, size_t size);
    LVoid readPNG(const LByte* data, size_t size);

    virtual LVoid* item() const;
    virtual LVoid* pixels() const;
    virtual const String& url() const;

private:
    HtmlView* m_image;
    LByte* m_pixels;
};
}

#endif
