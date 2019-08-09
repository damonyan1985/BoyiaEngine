#include "VirtualMachine.h"
#include "BoyiaCore.h"

extern LVoid* CompileScript(char* code);

namespace yanbo {
VirtualMachine::VirtualMachine()
    : m_vm(NULL)
{
}

VirtualMachine::~VirtualMachine()
{
}

LVoid VirtualMachine::compile(const String& script)
{
    m_vm = CompileScript((char*)script.GetBuffer());
}
}