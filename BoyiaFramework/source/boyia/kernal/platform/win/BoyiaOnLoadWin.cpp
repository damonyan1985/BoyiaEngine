#include "BoyiaOnLoadWin.h"
#include <stdio.h>

#ifndef  _WINDLL
#define _WINDLL
#endif // ! _WINDLL


void BoyiaOnLoadWin::foo()
{
	//do something...
	printf("hello world");
}