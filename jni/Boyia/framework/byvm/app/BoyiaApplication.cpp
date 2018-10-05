#include "BoyiaApplication.h"
#include "BoyiaCore.h"

#define DEFAULT_ACTIVITY_SIZE 20
namespace boyia
{
BoyiaApplication::BoyiaApplication()
    : m_stack(DEFAULT_ACTIVITY_SIZE)
    , m_vm(NULL)
{
}

LVoid BoyiaApplication::initApp()
{
	m_vm = InitVM();
}
}
