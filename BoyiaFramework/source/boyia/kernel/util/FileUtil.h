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
#include "KVector.h"

namespace util {
// android中的文件读写不能操作扩展卡中的内容，只能操作/data/data/com.boyia.app.shell下文件陆慕
class FileUtil {
public:
    static LVoid readFile(const String& fileName, String& content);
    static LVoid writeFile(const String& fileName, const String& content);
    static bool isExist(const char* path);
    static bool isDir(const char* path);
    static bool isFile(const char* path);
    static bool isSpecialDir(const char* path);
    static LVoid deleteFile(const char* path);
    static LInt createDir(const char* path);
    static LInt createDirs(const char* path);
    static LVoid printAllFiles(const char* path);
    static LVoid syncLoadUrl(const String& url, String& content);
    static const String& fileSchema();
    static LVoid listFiles(const String& path, KVector<String>& dirs, KVector<String>& files);
};
}

using util::FileUtil;
#endif
