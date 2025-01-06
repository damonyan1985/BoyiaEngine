// boyia.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include "BoyiaRuntime.h"
#include "FileUtil.h"

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
        //const char* filename = "E:\\work\\project\\BoyiaEngine\\BoyiaTools\\test\\project\\apps\\sdk\\Util.boyia";
        std::cout << filename << std::endl;
        
        String content;
        FileUtil::readFile(_CS(filename), content);

        std::cout << content << std::endl;
        runtime.compile(content);
    }

    
    CacheVMCode(runtime.vm());

    std::cout << "Build Complete!\n" << argc;
    return 0;
}
