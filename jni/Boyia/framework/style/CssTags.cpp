/*
 * CssTags.cpp
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#include "CssTags.h"
#include "StringUtils.h"

namespace util
{
CssTags* CssTags::s_cssTags = NULL;

CssTags::CssTags()
    : m_map(CssTags::TAGEND)
{	
}

CssTags::~CssTags()
{	
}

CssTags* CssTags::getInstance()
{
	if (s_cssTags == NULL)
	{
		s_cssTags = new CssTags();
        /*
         * Initializing Css tokens.
         */
		s_cssTags->defineInitialSymbol(_CS("margin-top"), MARGIN_TOP);
		s_cssTags->defineInitialSymbol(_CS("margin-left"), MARGIN_LEFT);
		s_cssTags->defineInitialSymbol(_CS("border-style"), BORDER_STYLE);
		s_cssTags->defineInitialSymbol(_CS("border-color"), BORDER_COLOR);
		s_cssTags->defineInitialSymbol(_CS("border-top-color"), BORDER_TOP_COLOR);
		s_cssTags->defineInitialSymbol(_CS("border-top-style"), BORDER_LEFT_STYLE);
		s_cssTags->defineInitialSymbol(_CS("border-left-color"), BORDER_LEFT_COLOR );
		s_cssTags->defineInitialSymbol(_CS("border-left-style"), BORDER_LEFT_STYLE);
		s_cssTags->defineInitialSymbol(_CS("border-right-color"), BORDER_RIGHT_COLOR);
		s_cssTags->defineInitialSymbol(_CS("border-right-style"), BORDER_RIGHT_STYLE);
		s_cssTags->defineInitialSymbol(_CS("border-bottom-color"), BORDER_BOTTOM_COLOR);
		s_cssTags->defineInitialSymbol(_CS("border-bottom-style"), BORDER_BOTTOM_STYLE);
		s_cssTags->defineInitialSymbol(_CS("position"), POSITION);
		s_cssTags->defineInitialSymbol(_CS("left"), LEFT);
		s_cssTags->defineInitialSymbol(_CS("top"), TOP);
		s_cssTags->defineInitialSymbol(_CS("background"), BACKGROUND);
		s_cssTags->defineInitialSymbol(_CS("background-color"), BACKGROUND_COLOR);
		s_cssTags->defineInitialSymbol(_CS("color"), COLOR);
        s_cssTags->defineInitialSymbol(_CS("margin"), MARGIN);
        s_cssTags->defineInitialSymbol(_CS("font-size"), FONT_SIZE);
        s_cssTags->defineInitialSymbol(_CS("font-style"), FONT_STYLE);
        s_cssTags->defineInitialSymbol(_CS("font-weight"), FONT_WEIGHT);
        s_cssTags->defineInitialSymbol(_CS("text-align"), TEXT_ALIGN);
        s_cssTags->defineInitialSymbol(_CS("border-bottom"), BORDER_BOTTOM);
        s_cssTags->defineInitialSymbol(_CS("border-bottom-width"), BORDER_BOTTOM_WIDTH);
        s_cssTags->defineInitialSymbol(_CS("border-top"), BORDER_TOP);
        s_cssTags->defineInitialSymbol(_CS("padding"), PADDING);
        s_cssTags->defineInitialSymbol(_CS("padding-top"), PADDING_TOP);
        s_cssTags->defineInitialSymbol(_CS("padding-left"), PADDING_LEFT);
        s_cssTags->defineInitialSymbol(_CS("padding-bottom"), PADDING_BOTTOM);
        s_cssTags->defineInitialSymbol(_CS("padding-right"), PADDING_RIGHT);
        s_cssTags->defineInitialSymbol(_CS("border"), BORDER);
        s_cssTags->defineInitialSymbol(_CS("background-image"), BACKGROUND_IMAGE);
        s_cssTags->defineInitialSymbol(_CS("display"), DISPLAY);
        s_cssTags->defineInitialSymbol(_CS("float"), FLOAT);
        s_cssTags->defineInitialSymbol(_CS("text-align"), TEXT_ALIGN);
        s_cssTags->defineInitialSymbol(_CS("width"), WIDTH);
        s_cssTags->defineInitialSymbol(_CS("height"), HEIGHT);
        s_cssTags->defineInitialSymbol(_CS("scale"), SCALE);
        s_cssTags->defineInitialSymbol(_CS("z-index"), Z_INDEX);
        s_cssTags->defineInitialSymbol(_CS("focusable"), FOCUSABLE);
        s_cssTags->defineInitialSymbol(_CS("flex-direction"), FLEX_DIRECTION);

        s_cssTags->m_map.sort();
	}
	
	return s_cssTags;
}

void CssTags::defineInitialSymbol(const String& cssText, LInt cssType)
{
	// Use Tagmap accelerate search speed
	m_map.put(cssText, cssType);
}

void CssTags::clear()
{
    m_map.clear();	
}

void CssTags::destroyInstance()
{
	if (s_cssTags != NULL)
	{
	    delete s_cssTags;
	    s_cssTags = NULL;
	}
}

LInt CssTags::symbolAsInt(LUint hash)
{
	return m_map.get(hash);
}

LUint CssTags::genIdentify(const String& key)
{
	return m_map.genKey(key);
}
}
