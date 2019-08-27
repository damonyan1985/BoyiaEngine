#include "AppVM.h"
#include "BoyiaCore.h"

extern LVoid* CompileScript(char* code);

namespace yanbo {
AppVM::AppVM()
    : m_vm(InitVM())
{
}

AppVM::~AppVM()
{
    DestroyVM(m_vm);
}

LVoid AppVM::compile(const String& script)
{
    m_vm = CompileScript((char*)script.GetBuffer());
}

LVoid* AppVM::useVM()
{
    ChangeVM(m_vm);
}
}