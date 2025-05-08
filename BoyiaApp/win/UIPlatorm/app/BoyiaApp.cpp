#define _WINDLL
#include "BoyiaConsole.h"
#include "window/BoyiaWindow.h"
//#include "http/BoyiaHttpEngine.h"
//#include "PlatformLib.h"
#include "BoyiaOnLoadWin.h"
#include <dbghelp.h>
#include <stdlib.h>
#include <new.h>
#include <signal.h>
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:windows")

using namespace yanbo;

extern BoyiaApp* GetBoyiaApp();
int BoyiaWinApi BoyiaWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow)
{
    BoyiaConsole console;
    BoyiaUIEngine uiEngine;

    BoyiaApp* ptApp = GetBoyiaApp();
    ptApp->InitInstance(&uiEngine, hInstance, nCmdShow);
    ptApp->Run();
    return 0;
}