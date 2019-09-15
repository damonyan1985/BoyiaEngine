#ifndef ImageLoader_h
#define ImageLoader_h

#include "BoyiaLoader.h"

namespace yanbo {

class ImageClient {
public:
    virtual LVoid setData(const String& data);
}

class ImageLoader {
public:
    static ImageLoader* instance();
    LVoid loadImage(const String& url, ImageClient* client);

private:
    BoyiaLoader m_loader;
};
}

#endif