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
// android中的文件读写不能操作扩展卡中的内容，只能操作/data/data/com.boyia.app.shell下文件目录
typedef LVoid* FileHandle;
class FileUtil {
public:
    static LVoid readFile(In const String& fileName, Out String& content);
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
    static LVoid listAllFiles(const String& path, KVector<String>& files);
    static LVoid getCurrentAbsolutePath(In const String& relativePath, Out String& absolutePath);
    static LVoid getAbsoluteFilePath(In const String& absolutePath, In const String& relativeFilePath, Out String& absoluteFilePath);
    static bool IsAbsolutePath(const String& path);
    static LVoid* mmap(FileHandle handle, LSizeT size, LOffset offset, FileHandle* extra);
    static LBool munmap(LVoid* data, LSizeT size, FileHandle* extra);
};
}

using util::FileUtil;
#endif
