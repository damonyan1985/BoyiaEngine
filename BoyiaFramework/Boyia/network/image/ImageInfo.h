#ifndef ImageInfo_h
#define ImageInfo_h

#include "UtilString.h"

namespace yanbo {
class ImageInfo {
public:
    enum ImageType {
        kImageNone,
        kImageJpeg,
        kImagePng,
        kImageGif
    };
    ImageInfo();
    ImageInfo(const ImageInfo& info);

    LVoid decodeImage(const String& data);

private:
    LInt getType(const char* data);
    LVoid readJPEG(const LByte* data, size_t size);
    LVoid readPNG(const LByte* data, size_t size);

public:
    LInt width;
    LInt height;
    LByte* pixels;
};
}

#endif