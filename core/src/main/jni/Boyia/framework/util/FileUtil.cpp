#include "FileUtil.h"
#include "UtilString.h"
#include "SalLog.h"
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

namespace util
{
LVoid FileUtil::readFile(const String& fileName, String& content)
{
    FILE* file = fopen(GET_STR(fileName), "r");
    fseek(file, 0, SEEK_END);
    int len = ftell(file); //获取文件长度
    LInt8* buf = new LInt8[len+1];
    LMemset(buf, 0, len + 1);
    rewind(file);
    fread(buf, sizeof(char), len, file);
    fclose(file);

    // shallow copy
    content.Copy(_CS(buf), LTrue, len);
}

bool FileUtil::isExist(const char* path)
{
	return access(path, F_OK) == 0;
}

bool FileUtil::isDir(const char* path)
{
	struct stat statbuf;
	if (0 == lstat(path, &statbuf))// lstat返回文件的信息，文件信息存放在stat结构中
	{
	    return S_ISDIR(statbuf.st_mode) != 0;// S_ISDIR宏，判断文件类型是否为目录
	}

	return false;
}

bool FileUtil::isFile(const char* path)
{
	struct stat statbuf;
	if (0 == lstat(path, &statbuf))
	{
		return S_ISREG(statbuf.st_mode) != 0;//判断文件是否为常规文件
	}

	return false;
}

bool FileUtil::isSpecialDir(const char* path)
{
	return strcmp(path, ".") == 0 || strcmp(path, "..") == 0;
}

LVoid FileUtil::deleteFile(const char* path)
{
    DIR *dir;
    dirent *dirInfo;
    if (isFile(path))
    {
	    remove(path);
	    return;
    }

    if (isDir(path))
    {
        if ((dir = opendir(path)) == NULL)
            return;
        while ((dirInfo = readdir(dir)) != NULL)
        {
            //get_file_path(path, dirInfo->d_name, filePath);
            if (isSpecialDir(dirInfo->d_name))
                continue;

            String filePath = _CS(path);
            LInt len = filePath.GetLength();
            if ('/' != filePath[len-1])
            {
            	filePath += '/';
            }
            filePath += _CS("/");
            filePath += _CS(dirInfo->d_name);
            deleteFile(GET_STR(filePath));
            rmdir(GET_STR(filePath));
        }
    }
}

LInt FileUtil::createDirs(const char* path)
{
	CString dirName = path;
	LInt len = dirName.GetLength();
	if ('/' != dirName[len-1]) {
		dirName += '/';
		++len;
	}

	for (LInt i = 1; i < len; ++i)
	{
		if ('/' == dirName[i])
		{
			dirName[i] = '\0';
			if (access(dirName.GetBuffer(), F_OK) != 0)
			{
				if (mkdir(dirName.GetBuffer(), 0777) == -1)
				{
					return -1;
				}
			}

			dirName[i] = '/';
		 }
  }

  return 0;
}

LVoid FileUtil::printAllFiles(const char* path)
{
	KFORMATLOG("FileUtil::printAllFiles filePath=%s", path);
	DIR *d;
	struct dirent *file;
	struct stat sb;

	if (!(d = opendir(path)))
	{
		KFORMATLOG("FileUtil::printAllFiles error opendir %s!!!", path);
		return;
	}

	while ((file = readdir(d)) != NULL)
	{
		//if (strncmp(file->d_name, ".", 1) == 0)
		if (isSpecialDir(file->d_name))
			continue;
		//strcpy(filename[len++], file->d_name);
		CString filePath = path;
		LInt len = filePath.GetLength();
		if ('/' != filePath[len-1])
		{
			filePath += '/';
		}

		filePath += file->d_name;
		const char* subPath = GET_STR(filePath);
		if (isDir(subPath))
		{
			printAllFiles(subPath);
		}
		else
		{
			KFORMATLOG("FileUtil::printAllFiles filePath=%s", subPath);
		}
	}

	closedir(d);
}
}
