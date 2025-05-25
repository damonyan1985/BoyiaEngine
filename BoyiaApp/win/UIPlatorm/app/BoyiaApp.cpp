#include "BoyiaConsole.h"
#include "window/BoyiaWindow.h"

#include "BoyiaOnLoadWin.h"
#include "BoyiaAppWindow.h"
//#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:windows")

using namespace yanbo;

int BoyiaWinApi BoyiaWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow)
{
    BoyiaConsole console;
    BoyiaUIEngine uiEngine;

    BoyiaAppImpl app;
    app.InitInstance(&uiEngine, nCmdShow);
    app.Run();
    return 0;
}