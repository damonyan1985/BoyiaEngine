#ifndef LColor_h
#define LColor_h

#include "LGdi.h"

namespace util
{

#define COLOR_WHITE                     (LUint)0xffffffff
#define COLOR_GRAY                      (LUint)0xff808080
#define COLOR_DARKGRAY                  (LUint)0xff404040
#define COLOR_BLACK                     (LUint)0xff000000
#define COLOR_RED                       (LUint)0xffff0000
#define COLOR_PINK                      (LUint)0xffffafaf
#define COLOR_ORANGE                    (LUint)0xffffc800
#define COLOR_YELLOW                    (LUint)0xffffff00
#define COLOR_GREEN                     (LUint)0xff00ff00
#define COLOR_MAGENTA                   (LUint)0xffff00ff
#define COLOR_CYAN                      (LUint)0xff00ffff
#define COLOR_BLUE                      (LUint)0xff0000ff
#define COLOR_LINKCOLOR                 COLOR_BLUE // (LInt)0x005bae
#define COLOR_RADIOGREEN                (LUint)0xff169133
#define COLOR_LIGHTGRAY                 (LUint)0xffc0c0c0

class LColor
{
public:
	static LUint rgb(LInt red, LInt green, LInt blue, LInt alpha = 0xff);
	static LUint parseRgbString(const String& rgbString);
	static LRgb  parseRgbInt(LUint rgbValue);
	static LRgb  parseArgbInt(LUint argbValue);
};

}
#endif
