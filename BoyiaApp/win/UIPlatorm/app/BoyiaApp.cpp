#include "window/BoyiaWindow.h"
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:windows")

using namespace yanbo;
extern BoyiaApp* tfxGetApp();
int BoyiaWinApi BoyiaWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
)
{
	BoyiaApp *ptApp = tfxGetApp();
	ptApp->InitInstance(hInstance, nCmdShow);
	ptApp->run();
	return 0;
}