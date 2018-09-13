#ifndef FontFreeType_h
#define FontFreeType_h

#include "PlatformLib.h"
#include "UtilString.h"
#include <ft2build.h>
#include <freetype/freetype.h>

namespace yanbo
{
class FontFreeType
{
public:
	FontFreeType();
	LVoid initFont(const String& buffer);

private:
	FT_Library m_ftLibrary;
	FT_Face    m_face;
};
}

#endif
