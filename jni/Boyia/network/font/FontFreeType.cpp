#include "FontFreeType.h"
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

namespace yanbo
{
FontFreeType::FontFreeType()
{
}

LVoid FontFreeType::initFont(const String& buffer)
{
	// init font library
	if (FT_Init_FreeType(&m_ftLibrary))
	{
		return;
	}

	if (FT_New_Memory_Face(m_ftLibrary,
			(const FT_Byte*)buffer.GetBuffer(),
			(FT_Long)buffer.GetLength(),
			0,
			&m_face))
	{
		return;
	}

	if (FT_Select_Charmap(m_face, FT_ENCODING_UNICODE))
	{
		return;
	}
}
}
