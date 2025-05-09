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
    BoyiaOnLoadWin();
    ~BoyiaOnLoadWin();

    void setContextWin(HWND hwnd, int width, int height);
    void repaint();
 
    void connectServer();
    void handleTouchEvent(int type, int x, int y);
    void cacheCode();

private:
    void initEngine();
    void destroyEngine();
};

#endif