#include "AppVM.h"
#include "BoyiaCore.h"

extern LVoid* CompileScript(char* code);

namespace yanbo {
AppVM::AppVM()
    : m_vm(NULL)
{
}

AppVM::~AppVM()
{
}

LVoid AppVM::compile(const String& script)
{
    m_vm = CompileScript((char*)script.GetBuffer());
}
}