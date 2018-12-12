/*
============================================================================
 Name        : FileUtil.h
 Author      : yanbo
 Version     : FileUtil v0.0.1
 Copyright   : All Copyright Reserved
 Date        : 2018-10-13
 Description : Support Original
============================================================================
*/

#ifndef FileUtil_h
#define FileUtil_h

#include "UtilString.h"

namespace util
{
class FileUtil
{
public:
    static LVoid readFile(const String& fileName, String& content);
    static bool isExist(const char* path);
    static bool isDir(const char* path);
    static bool isFile(const char* path);
    static bool isSpecialDir(const char* path);
    static LVoid deleteFile(const char* path);
    static LInt createDirs(const char* path);
    static LVoid printAllFiles(const char* path);
};
}

using util::FileUtil;
#endif
