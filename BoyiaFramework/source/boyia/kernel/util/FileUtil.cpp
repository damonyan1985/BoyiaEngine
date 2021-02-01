#include "FileUtil.h"

#include "PlatformBridge.h"
#include "SalLog.h"
#include "UtilString.h"
#include <stdio.h>
#include <string.h>
#if ENABLE(BOYIA_ANDROID)
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#elif ENABLE(BOYIA_WINDOWS)
#include "CharConvertor.h"
#include <io.h>
#include <windows.h>
#endif

const String kSourcePrefix(_CS("boyia://"), LTrue, 8);

namespace util {
const String& FileUtil::fileSchema()
{
    return kSourcePrefix;
}

LVoid FileUtil::readFile(const String& fileName, String& content)
{
    FILE* file = fopen(GET_STR(fileName), "rb");
    if (!file) {
        return;
    }
    fseek(file, 0, SEEK_END);
    int len = ftell(file); //获取文件长度
    LInt8* buf = NEW_BUFFER(LInt8, len + 1);
    LMemset(buf, 0, len + 1);
    rewind(file);
    fread(buf, sizeof(char), len, file);
    fclose(file);

    // shallow copy
    content.Copy(_CS(buf), LTrue, len);
}

LVoid FileUtil::writeFile(const String& fileName, const String& content)
{
    FILE* file = fopen(GET_STR(fileName), "wb");
    if (!file) {
        return;
    }

    fwrite(GET_STR(content), 1, content.GetLength(), file);
    fclose(file);
}

LVoid FileUtil::syncLoadUrl(const String& url, String& content)
{
    if (url.StartWith(kSourcePrefix)) {
        String sourceUrl = url.Mid(kSourcePrefix.GetLength());
        String sourcePath = _CS(yanbo::PlatformBridge::getAppRoot()) + sourceUrl;
        FileUtil::readFile(sourcePath, content);
    }
}

bool FileUtil::isExist(const char* path)
{
#if ENABLE(BOYIA_ANDROID)
    return access(path, F_OK) == 0;
#elif ENABLE(BOYIA_WINDOWS)
    return _access(path, _A_NORMAL) != -1;
#endif
}

bool FileUtil::isDir(const char* path)
{
#if ENABLE(BOYIA_ANDROID)
    struct stat statbuf;
    if (0 == lstat(path, &statbuf)) // lstat返回文件的信息，文件信息存放在stat结构中
    {
        return S_ISDIR(statbuf.st_mode) != 0; // S_ISDIR宏，判断文件类型是否为目录
    }
#elif ENABLE(BOYIA_WINDOWS)
    wstring wpath = yanbo::CharConvertor::CharToWchar(path);
    if (GetFileAttributes(wpath.c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
        return true;
    }
#endif

    return false;
}

bool FileUtil::isFile(const char* path)
{
#if ENABLE(BOYIA_ANDROID)
    struct stat statbuf;
    if (0 == lstat(path, &statbuf)) {
        return S_ISREG(statbuf.st_mode) != 0; //判断文件是否为常规文件
    }

    return false;
#elif ENABLE(BOYIA_WINDOWS)
    return !isDir(path);
#endif
}

bool FileUtil::isSpecialDir(const char* path)
{
    return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}

#if ENABLE(BOYIA_WINDOWS)
static LVoid deleteFileWin(const wstring& path)
{
    WIN32_FIND_DATA data;

    HANDLE hFile = ::FindFirstFile(path.c_str(), &data);
    wstring fullPath = path;
    do {
        if ((!wcscmp(L".", data.cFileName)) || (!wcscmp(L"..", data.cFileName))) {
            continue;
        }

        fullPath += data.cFileName;
        if (data.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY) {
            deleteFileWin(fullPath);
        } else {
            ::DeleteFile(fullPath.c_str());
        }
    } while (::FindNextFile(hFile, &data));

    //::CloseHandle(hFile);
    ::FindClose(hFile);
    ::RemoveDirectory(path.c_str());
}
#endif

LVoid FileUtil::deleteFile(const char* path)
{
#if ENABLE(BOYIA_ANDROID)
    DIR* dir;
    dirent* dirInfo;
    if (isFile(path)) {
        remove(path);
        return;
    }

    if (isDir(path)) {
        if ((dir = opendir(path)) == kBoyiaNull) {
            return;
        }

        while ((dirInfo = readdir(dir)) != kBoyiaNull) {
            //get_file_path(path, dirInfo->d_name, filePath);
            if (isSpecialDir(dirInfo->d_name)) {
                continue;
            }

            String filePath = _CS(path);
            LInt len = filePath.GetLength();
            if ('/' != filePath[len - 1]) {
                filePath += '/';
            }
            filePath += _CS("/");
            filePath += _CS(dirInfo->d_name);
            deleteFile(GET_STR(filePath));
            rmdir(GET_STR(filePath));
        }
    }
#elif ENABLE(BOYIA_WINDOWS)
    if (!isExist(path)) {
        return;
    }
    wstring wpath = yanbo::CharConvertor::CharToWchar(path);
    if (isDir(path)) {
        deleteFileWin(wpath);
    } else {
        ::DeleteFile(wpath.c_str());
    }
#endif
}

LInt FileUtil::createDir(const char* path)
{
#if ENABLE(BOYIA_ANDROID)
    return mkdir(path, S_IRWXU);
#elif ENABLE(BOYIA_WINDOWS)
    wstring wpath = yanbo::CharConvertor::CharToWchar(path);
    return CreateDirectory(wpath.c_str(), kBoyiaNull);
#endif
}

LInt FileUtil::createDirs(const char* path)
{
#if ENABLE(BOYIA_ANDROID)
    CString dirName = path;
    LInt len = dirName.GetLength();
    if ('/' != dirName[len - 1]) {
        dirName += '/';
        ++len;
    }

    for (LInt i = 1; i < len; ++i) {
        if ('/' == dirName[i]) {
            dirName[i] = '\0';
            if (access(dirName.GetBuffer(), F_OK) != 0 && mkdir(dirName.GetBuffer(), 0777) == -1) {
                return -1;
            }

            dirName[i] = '/';
        }
    }

#elif ENABLE(BOYIA_WINDOWS)
#endif
    return 0;
}

LVoid FileUtil::printAllFiles(const char* path)
{
    BOYIA_LOG("FileUtil::printAllFiles filePath=%s", path);
#if ENABLE(BOYIA_ANDROID)
    DIR* d;
    struct dirent* file;
    struct stat sb;

    if (!(d = opendir(path))) {
        BOYIA_LOG("FileUtil::printAllFiles error opendir %s!!!", path);
        return;
    }

    while ((file = readdir(d)) != kBoyiaNull) {
        //if (strncmp(file->d_name, ".", 1) == 0)
        if (isSpecialDir(file->d_name))
            continue;
        //strcpy(filename[len++], file->d_name);
        CString filePath = path;
        LInt len = filePath.GetLength();
        if ('/' != filePath[len - 1]) {
            filePath += '/';
        }

        filePath += file->d_name;
        const char* subPath = GET_STR(filePath);
        if (isDir(subPath)) {
            printAllFiles(subPath);
        } else {
            BOYIA_LOG("FileUtil::printAllFiles final filePath=%s", subPath);
        }
    }

    closedir(d);
#elif ENABLE(BOYIA_WINDOWS)
#endif
}
}