#include "BoyiaApplication.h"
#include "BoyiaMemory.h"

#define DEFAULT_ACTIVITY_SIZE 20
namespace boyia
{
BoyiaApplication::BoyiaApplication()
    : m_actStack(DEFAULT_ACTIVITY_SIZE)
{
    m_vm = InitVM();
}

BoyiaApplication::~BoyiaApplication()
{
    DestroyVM(m_vm);
}

LVoid BoyiaApplication::resume()
{
    // Change Global VM To Run Current Application
    ChangeVM(m_vm);
}
}
