#include "BoyiaApplication.h"
#include "BoyiaMemory.h"

#define DEFAULT_ACTIVITY_SIZE 20
namespace boyia
{
BoyiaApplication::BoyiaApplication()
    : m_actStack(DEFAULT_ACTIVITY_SIZE)
    , m_vm(NULL)
{
}

BoyiaApplication::~BoyiaApplication()
{
	if (m_vm)
	{
		FAST_DELETE(m_vm);
	}
}

LVoid BoyiaApplication::initApp()
{
	m_vm = InitVM();
}

LVoid BoyiaApplication::resume()
{
	// Change Global VM To Run Current Application
	ChangeVM(m_vm);
}
}
