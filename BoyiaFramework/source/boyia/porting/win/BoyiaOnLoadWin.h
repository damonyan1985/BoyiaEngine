#ifndef BoyiaOnLoadWin_h
#define BoyiaOnLoadWin_h

#include <Windows.h>

#ifdef _WINDLL
#define BOYIA_PORTING_API __declspec(dllexport)
#else
#define BOYIA_PORTING_API __declspec(dllimport)
#endif

class BOYIA_PORTING_API BoyiaOnLoadWin
{
public:
    static void initEngine();
    static void destroyEngine();

	static void setContextWin(HWND hwnd, int width, int height);
    static void repaint();
 
    static void connectServer();
    static void handleTouchEvent(int type, int x, int y);
    static void cacheCode();
};

#endif