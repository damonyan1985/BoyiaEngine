#include "BoyiaRuntime.h"
#include "BoyiaCore.h"
#include "SalLog.h"
#include "Application.h"

extern LVoid CompileScript(char* code);

namespace boyia {
BoyiaRuntime::BoyiaRuntime(yanbo::Application* app)
    : m_app(app)
    , m_vm(InitVM(this))
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
    ChangeVM(m_vm);
    BOYIA_LOG("BoyiaRuntime---useVM---%s", "end");
}

LVoid* BoyiaRuntime::vm() const
{
    return m_vm;
}

yanbo::UIView* BoyiaRuntime::view() const
{
    return m_app->view();
}
}