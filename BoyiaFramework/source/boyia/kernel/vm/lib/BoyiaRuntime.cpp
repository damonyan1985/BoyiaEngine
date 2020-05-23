#include "BoyiaRuntime.h"
#include "BoyiaCore.h"
#include "SalLog.h"

extern LVoid CompileScript(char* code);
extern LVoid* CreateGC();
extern LVoid ChangeGC(LVoid* gc);

namespace boyia {
BoyiaRuntime::BoyiaRuntime()
    : m_vm(InitVM())
    , m_gc(CreateGC())
{
}

BoyiaRuntime::~BoyiaRuntime()
{
    DestroyVM(m_vm);
}

LVoid BoyiaRuntime::compile(const String& script)
{
    CompileScript((char*)script.GetBuffer());
}

LVoid BoyiaRuntime::useVM()
{
    BOYIA_LOG("BoyiaRuntime---useVM---%s", "ChangeGC");
    ChangeGC(m_gc);
    BOYIA_LOG("BoyiaRuntime---useVM---%s", "ChangeVM");
    ChangeVM(m_vm);
    BOYIA_LOG("BoyiaRuntime---useVM---%s", "end");
}

LVoid* BoyiaRuntime::vm() const
{
    return m_vm;
}
}