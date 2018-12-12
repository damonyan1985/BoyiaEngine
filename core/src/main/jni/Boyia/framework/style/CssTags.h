/*
 * CssTags.h
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#ifndef CssTags_h
#define CssTags_h

//#include "kmap.h"
#include "UtilString.h"
#include "TagMap.h"

namespace util
{
/**
 * Objects of this class holds the symbol table for CSS tags.
 * See this class for more info.
 */
class CssTags
{
public:
	//properties
	//properties that are inherited has value > STYLE_NULL.
	//properties that is not inherited has value < STYLE_NULL.
	enum CssType
	{
		TAGNONE = 0,
	    WIDTH,
	    HEIGHT,
	    DISPLAY,
	    FLEX_DIRECTION,
	    FLOAT,		
		BACKGROUND_IMAGE,
		PADDING,
		PADDING_TOP,
		PADDING_LEFT,
		PADDING_BOTTOM,
		PADDING_RIGHT,
		BORDER_TOP,
		BORDER_TOP_WIDTH,
		BORDER_BOTTOM,
		BORDER_BOTTOM_WIDTH,
		MARGIN,
		MARGIN_RIGHT,
		MARGIB_BOTTOM,
		MARGIN_TOP,
		MARGIN_LEFT,
		BORDER,
		BORDER_STYLE,
		BORDER_COLOR,
		BORDER_TOP_STYLE,
		BORDER_TOP_COLOR,
		BORDER_LEFT_STYLE,
		BORDER_LEFT_COLOR,
		BORDER_LEFT_WIDTH,
		BORDER_RIGHT_STYLE,
		BORDER_RIGHT_COLOR,
		BORDER_RIGHT_WIDTH,
		BORDER_BOTTOM_COLOR,
		BORDER_BOTTOM_STYLE,
		POSITION,
		LEFT,
		TOP,
		BACKGROUND,
		BACKGROUND_COLOR,
		Z_INDEX,
		FOCUSABLE,
		STYLE_NULL,
		COLOR,
		FONT_SIZE,
		FONT_STYLE,
		FONT_WEIGHT,
		TEXT_ALIGN,
		SCALE,
		TAGEND,
	};
	
public:
	static CssTags* getInstance();
	static void destroyInstance();
	
	void defineInitialSymbol(const String& cssText, LInt cssType);
	~CssTags();
	
	LInt symbolAsInt(LUint hash);
	void clear();
	
	LUint genIdentify(const String& key);

private:
	CssTags();	
	
private:
	static CssTags* s_cssTags;
	TagMap m_map;
};
}
#endif
