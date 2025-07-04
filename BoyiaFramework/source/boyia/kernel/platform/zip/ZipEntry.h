#ifndef ZipEntry_h
#define ZipEntry_h

#include "unzip.h"
#include "zip.h"
#include "zlib.h"
#include "UtilString.h"

namespace yanbo {
class ZipEntry {
public:
    static bool unzip(const char* src, const char* dest, const char* password);
    static bool zip(const String& src, const String& dest, const String& password);

private:
    static bool writeFile(unzFile& zipfile, char* name);
    static bool createDir(const char* path);
};
}

#endif