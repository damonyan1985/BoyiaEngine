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

// Exception handle for windows native program
static void InstallUnexceptedExceptionHandler();
//BoyiaHttpEngine* engine = new BoyiaHttpEngine(NULL);
extern BoyiaApp* tfxGetApp();
int BoyiaWinApi BoyiaWinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow)
{
    BoyiaConsole console;
    BoyiaOnLoadWin::networkInit();
    //HINSTANCE hInstLibrary = LoadLibrary(L"LibBoyia.dll");
    InstallUnexceptedExceptionHandler();
    //BoyiaOnLoadWin::foo();
    //char test[100];
    //util::LMemset(test, 0, 100);
    //engine->request("https://damonyan1985.github.io/app/boyia.json", NetworkBase::GET);
    BoyiaApp* ptApp = tfxGetApp();
    ptApp->InitInstance(hInstance, nCmdShow);
    ptApp->run();
    BoyiaOnLoadWin::networkDestroy();
    return 0;
}

static LONG BoyiaUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
    return EXCEPTION_EXECUTE_HANDLER;
}

static void BoyiaPureCallHandler()
{

}

static int BoyiaNewHandler(size_t size)
{
    return 0;
}

static void BoyiaInvalidParameterHandler(wchar_t const* expression,
    wchar_t const* function,
    wchar_t const* file,
    unsigned int line,
    uintptr_t pReserved)
{

}

static void BoyiaSigabrtHandler(int signal)
{

}

static void BoyiaSigintHandler(int signal)
{

}

static void BoyiaSigtermHandler(int signal)
{

}


static void BoyiaSigillHandler(int signal)
{

}

static void BoyiaTerminateHandler()
{

}

static void BoyiaUnexpectedHandler()
{

}


static void InstallUnexceptedExceptionHandler()
{
    // Crash handler Win32 API
    ::SetUnhandledExceptionFilter(BoyiaUnhandledExceptionFilter);
    // (CRT) Exception
    _set_purecall_handler(BoyiaPureCallHandler);
    _set_new_handler(BoyiaNewHandler);
    _set_invalid_parameter_handler(BoyiaInvalidParameterHandler);
    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
    
    signal(SIGABRT, BoyiaSigabrtHandler);
    signal(SIGINT, BoyiaSigintHandler);
    signal(SIGTERM, BoyiaSigtermHandler);
    signal(SIGILL, BoyiaSigillHandler);
    // C++ Exception
    set_terminate(BoyiaTerminateHandler);
    set_unexpected(BoyiaUnexpectedHandler);
}