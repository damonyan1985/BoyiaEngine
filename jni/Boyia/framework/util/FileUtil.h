#ifndef FileUtil_h
#define FileUtil_h

#include "UtilString.h"

namespace util
{
class FileUtil
{
public:
    static LVoid readFile(const String& fileName, String& content);
};
}

#endif
