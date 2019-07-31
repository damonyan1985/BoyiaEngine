#include "PlatformThreadWin.h"

namespace yanbo
{
PlatformThreadWin::PlatformThreadWin()
	: m_id(0) {

}

void PlatformThreadWin::start() {
	::CreateThread(NULL, 0, threadFunc, 0, 0, &m_id);
}

int PlatformThreadWin::getId() {
	return m_id;
}

DWORD WINAPI PlatformThreadWin::threadFunc(LPVOID ptr) {
	return 0;
}
}