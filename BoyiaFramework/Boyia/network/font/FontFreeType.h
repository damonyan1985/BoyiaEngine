#ifndef FontFreeType_h
#define FontFreeType_h

#include "PlatformLib.h"
#include "UtilString.h"
#include <GLES3/gl3.h>
#include <ft2build.h>
#include <freetype/freetype.h>

namespace yanbo {

class FTCharacter {
public:
    FTCharacter();
    ~FTCharacter();

    GLuint texID;
    LInt width;
    LInt height;
};

class FontFreeType {
public:
    FontFreeType();
    LVoid initFont(const String& buffer, float fontSize);
    GLuint loadChar(LUint64 ch);

private:
    static FT_Library s_ftLibrary;
    FT_Face m_face;
    FTCharacter m_chars[1 << 16];
};
}

#endif
