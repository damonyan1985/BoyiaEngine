#ifndef BoyiaOnLoadWin_h
#define BoyiaOnLoadWin_h

#ifdef _WINDLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

class DLL_API BoyiaOnLoadWin
{
public:
	static void foo();
};

#endif