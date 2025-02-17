// boyia.cpp : Boyia编译器入口
// 编译提示 : boyia.exe test1.boyia test2.boyia test3.boyia
//

#include <iostream>
#include <windows.h>
#include <shlwapi.h>
#include "BoyiaRuntime.h"
#include "FileUtil.h"

#pragma comment(lib, "shlwapi.lib")

// Such as execute E:\work\project\BoyiaEngine\BoyiaTools\boyia-ide\boyia\boyia\x64\Debug>boyia.exe ..\..\..\script\build.boyia
int main(int argc, char** argv)
{
    boyia::BoyiaRuntime runtime;
    runtime.init();
    if (argc <= 1) {
        std::cout << "No args in command!\n" << argc;
        return 0;
    }

    for (LInt i = 1; i < argc; i++) {
        const char* filename = argv[i];
        //const char* filename = "..\\..\\script\\build.boyia";
        String path;
        if (!FileUtil::IsAbsolutePath(_CS(filename))) {
            FileUtil::getCurrentAbsolutePath(_CS(filename), path);
        } else {
            path = _CS(filename);
        }
        //const char* filename = "E:\\work\\project\\BoyiaEngine\\BoyiaTools\\test\\project\\apps\\sdk\\Util.boyia";
        std::cout << GET_STR(path) << std::endl;

        /*String pathEx = _CS("..\\UtilEx.boyia");
        FileUtil::getAbsoluteFilePath(path, pathEx, pathEx);
        std::cout << GET_STR(pathEx) << std::endl;*/
        
        //String content;
        //FileUtil::readFile(_CS(filename), content);

        //std::cout << content << std::endl;
        runtime.compileFile(path);
    }

    
    CacheVMCode(runtime.vm());

    std::cout << "Build Complete!\n" << argc;
    return 0;
}
