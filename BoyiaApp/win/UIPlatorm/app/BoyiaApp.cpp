#include "window/BoyiaWindow.h"
#include "http/BoyiaHttpEngine.h"
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:windows")

using namespace yanbo;

BoyiaHttpEngine* engine = new BoyiaHttpEngine(NULL);
extern BoyiaApp* tfxGetApp();
int BoyiaWinApi BoyiaWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine,
	int nCmdShow
)
{
	engine->request("https://damonyan1985.github.io/app/boyia.json", NetworkBase::GET);
	BoyiaApp *ptApp = tfxGetApp();
	ptApp->InitInstance(hInstance, nCmdShow);
	ptApp->run();
	return 0;
}