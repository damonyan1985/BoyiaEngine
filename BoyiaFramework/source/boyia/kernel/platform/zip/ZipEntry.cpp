#include "ZipEntry.h"
#include "FileUtil.h"
//#include <dirent.h>
#include <stdio.h>
#include <string.h>
//#include <sys/stat.h>
//#include <unistd.h>

#define dir_delimter '/'
#define MAX_FILENAME 512
#define READ_SIZE 8192

namespace yanbo {
bool ZipEntry::unzip(const char* src, const char* dest, const char* password)
{
    unzFile zipfile = unzOpen(src);
    if (zipfile == kBoyiaNull)
        return false;

    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
        unzClose(zipfile);
        return false;
    }
    char read_buffer[READ_SIZE] = { 0 };

    for (uLong i = 0; i < global_info.number_entry; ++i) {
        unz_file_info64 file_info64;
        char filename[MAX_FILENAME] = { 0 };
        unzGetCurrentFileInfo64(zipfile, &file_info64, filename,
            sizeof(filename), kBoyiaNull, 0, kBoyiaNull, 0);

        const size_t filename_length = strlen(filename);
        char name[256] = { 0 };
        if (filename[filename_length - 1] == dir_delimter) {
            sprintf(name, "%s/%s", dest, filename);
            createDir(name);
        } else {
            if (password && unzOpenCurrentFilePassword(zipfile, password) != UNZ_OK) {
                unzClose(zipfile);
                return false;
            }

            if (!password && unzOpenCurrentFile(zipfile) != UNZ_OK) {
                unzClose(zipfile);
                return false;
            }
            sprintf(name, "%s/%s", dest, filename);

            if (writeFile(zipfile, name) < 0) {
                unzCloseCurrentFile(zipfile);
                unzClose(zipfile);
                return false;
            }
            //setFileTime(name, file_info64.dosDate);
        }
        unzCloseCurrentFile(zipfile);

        if ((i + 1) < global_info.number_entry && unzGoToNextFile(zipfile) != UNZ_OK) {
            unzClose(zipfile);
            return false;
        }
    }

    unzClose(zipfile);
    return true;
}

bool ZipEntry::createDir(const char* path)
{
    char dirName[256] = { 0 };
    strcpy(dirName, path);
    int len = strlen(dirName);
    for (int i = 1; i < len; i++) {
        if (dirName[i] != '/') {
            continue;
        }
        dirName[i] = 0;
        if (!FileUtil::isExist(dirName) && FileUtil::createDir(dirName) == -1) {
            return false;
        }
        dirName[i] = '/';
    }

    return true;
}

bool ZipEntry::writeFile(unzFile& zipfile, char* name)
{
    if (name == kBoyiaNull) {
        return false;
    }
    createDir(name);
    FILE* out = fopen(name, "wb");
    if (out == kBoyiaNull) {
        return false;
    }

    char read_buffer[READ_SIZE] = { 0 };
    int error = UNZ_OK;
    do {
        error = unzReadCurrentFile(zipfile, read_buffer, READ_SIZE);
        if (error < 0)
            return false;

        if (error > 0) {
            fwrite(read_buffer, error, 1, out);
        }
    } while (error > 0);

    fclose(out);
    out = kBoyiaNull;
    return true;
}
}