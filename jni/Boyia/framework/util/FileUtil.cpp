#include "FileUtil.h"
#include <stdio.h>

namespace util
{
LVoid FileUtil::readFile(const String& fileName, String& content)
{
	const char* strName = (const char*)fileName.GetBuffer();
    FILE* file = fopen(strName, "r");
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
}
