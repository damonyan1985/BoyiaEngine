#ifndef ZipEntry_h
#define ZipEntry_h

#include "unzip.h"
#include "zip.h"
#include "zlib.h"

namespace yanbo {
class ZipEntry {
public:
    static int unzip(const char* src, const char* dest, const char* password);

private:
    static int writeFile(unzFile& zipfile, char* name);
    static int createDir(const char* path);
};
}

#endif