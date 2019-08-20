#include "FontFreeType.h"
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

namespace yanbo {
FT_Library FontFreeType::s_ftLibrary;

FTCharacter::FTCharacter()
    : texID(0)
{
}

FTCharacter::~FTCharacter()
{
    if (texID) {
        glDeleteTextures(1, &texID);
    }
}

FontFreeType::FontFreeType()
{
}

LVoid FontFreeType::initFont(const String& buffer, float fontSize)
{
    // init font library
    if (FT_Init_FreeType(&s_ftLibrary)) {
        return;
    }

    if (FT_New_Memory_Face(s_ftLibrary,
            (const FT_Byte*)buffer.GetBuffer(),
            (FT_Long)buffer.GetLength(),
            0,
            &m_face)) {
        return;
    }

    if (FT_Select_Charmap(m_face, FT_ENCODING_UNICODE)) {
        return;
    }

    int dpi = 72;
    int fontSizePoints = (int)(64.f * fontSize * 1); // 1 temp factor
    FT_Set_Char_Size(m_face, fontSizePoints, fontSizePoints, dpi, dpi);
}

GLuint FontFreeType::loadChar(LUint64 ch)
{
    if (m_chars[ch].texID) {
        return m_chars[ch].texID;
    }

    FTCharacter* chTex = &m_chars[ch];

    if (FT_Load_Char(m_face, ch, FT_LOAD_RENDER | FT_LOAD_NO_AUTOHINT)) {
        return 0;
    }

    FT_Glyph glyph;
    if (FT_Get_Glyph(m_face->glyph, &glyph)) {
        return 0;
    }

    FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_LCD);

    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    int width = bitmap.width;
    int height = bitmap.rows;

    chTex->width = width;
    chTex->height = height;

    glGenTextures(1, &chTex->texID);
    glBindTexture(GL_TEXTURE_2D, chTex->texID);

    char* buffer = new char[width * height * 4];
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            unsigned char color = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
            buffer[(4 * i + (height - j - 1) * width * 4)] = color; //0xff;
            buffer[(4 * i + (height - j - 1) * width * 4) + 1] = color; //0xff;
            buffer[(4 * i + (height - j - 1) * width * 4) + 2] = color; //0xff;
            buffer[(4 * i + (height - j - 1) * width * 4) + 3] = color; //_vl;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return chTex->texID;
}
}
