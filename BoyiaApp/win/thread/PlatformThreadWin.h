#ifndef PlatformThreadWin_h
#define PlatformThreadWin_h

#include "BaseThread.h"
#include<windows.h>

namespace yanbo
{
class PlatformThreadWin : public BaseThread {
public:
	PlatformThreadWin();
	void start();
	virtual int getId();

private:
	static DWORD WINAPI threadFunc(LPVOID ptr);

	DWORD m_id;
};
}

#endif
