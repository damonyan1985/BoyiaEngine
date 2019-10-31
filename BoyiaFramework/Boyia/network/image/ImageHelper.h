#ifndef ImageHelper_h
#define ImageHelper_h

#include "UtilString.h"

namespace yanbo {
class ImageHelper {
public:
    enum ImageType {
        kImageNone,
        kImageJpeg,
        kImagePng,
        kImageGif
    };
    static LVoid decodeImage(const String& data);

private:
    static LInt getType(const char* data);
    static LVoid readJPEG(const LByte* data, size_t size);
    static LVoid readPNG(const LByte* data, size_t size);
    static LVoid sendMessage(LInt width, LInt height, LVoid* pixels);
};
}

#endif