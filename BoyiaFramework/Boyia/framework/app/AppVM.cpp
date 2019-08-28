#include "AppVM.h"
#include "BoyiaCore.h"
#include "SalLog.h"

extern LVoid CompileScript(char* code);
extern LVoid* CreateGC();
extern LVoid ChangeGC(LVoid* gc);

namespace yanbo {
AppVM::AppVM()
    : m_vm(InitVM())
    , m_gc(CreateGC())
{
}

AppVM::~AppVM()
{
    DestroyVM(m_vm);
}

LVoid AppVM::compile(const String& script)
{
    CompileScript((char*)script.GetBuffer());
}

LVoid AppVM::useVM()
{
    BOYIA_LOG("AppVM---useVM---%s", "ChangeGC");
    ChangeGC(m_gc);
    BOYIA_LOG("AppVM---useVM---%s", "ChangeVM");
    ChangeVM(m_vm);
    BOYIA_LOG("AppVM---useVM---%s", "end");
}
}