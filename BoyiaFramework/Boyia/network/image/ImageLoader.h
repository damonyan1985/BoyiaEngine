#ifndef ImageLoader_h
#define ImageLoader_h

#include "BoyiaLoader.h"

namespace yanbo {

class ImageClient {
public:
    virtual ~ImageClient();
    virtual LVoid setData(const String& data) = 0;
};

class ImageLoader {
public:
    static ImageLoader* instance();
    LVoid loadImage(const String& url, ImageClient* client);

private:
    BoyiaLoader m_loader;
};
}

#endif