/*
 * StyleTags.cpp
 *
 *  Created on: 2011-6-23
 *      Author: yanbo
 */

#include "StyleTags.h"
#include "StringUtils.h"

namespace util {

StyleTags* StyleTags::s_tags = kBoyiaNull;

StyleTags::StyleTags()
{
}

StyleTags::~StyleTags()
{
}

StyleTags* StyleTags::getInstance()
{
    if (!s_tags) {
        s_tags = new StyleTags();
        /*
         * Initializing Css tokens.
         */
        s_tags->defineInitialSymbol(_CS("align"), ALIGN);
        s_tags->defineInitialSymbol(_CS("margin-top"), MARGIN_TOP);
        s_tags->defineInitialSymbol(_CS("margin-left"), MARGIN_LEFT);
        s_tags->defineInitialSymbol(_CS("border-style"), BORDER_STYLE);
        s_tags->defineInitialSymbol(_CS("border-color"), BORDER_COLOR);
        s_tags->defineInitialSymbol(_CS("border-top-color"), BORDER_TOP_COLOR);
        s_tags->defineInitialSymbol(_CS("border-top-style"), BORDER_LEFT_STYLE);
        s_tags->defineInitialSymbol(_CS("border-left-color"), BORDER_LEFT_COLOR);
        s_tags->defineInitialSymbol(_CS("border-left-style"), BORDER_LEFT_STYLE);
        s_tags->defineInitialSymbol(_CS("border-right-color"), BORDER_RIGHT_COLOR);
        s_tags->defineInitialSymbol(_CS("border-right-style"), BORDER_RIGHT_STYLE);
        s_tags->defineInitialSymbol(_CS("border-bottom-color"), BORDER_BOTTOM_COLOR);
        s_tags->defineInitialSymbol(_CS("border-bottom-style"), BORDER_BOTTOM_STYLE);
        s_tags->defineInitialSymbol(_CS("position"), POSITION);
        s_tags->defineInitialSymbol(_CS("left"), LEFT);
        s_tags->defineInitialSymbol(_CS("top"), TOP);
        s_tags->defineInitialSymbol(_CS("background"), BACKGROUND);
        s_tags->defineInitialSymbol(_CS("background-color"), BACKGROUND_COLOR);
        s_tags->defineInitialSymbol(_CS("color"), COLOR);
        s_tags->defineInitialSymbol(_CS("margin"), MARGIN);
        s_tags->defineInitialSymbol(_CS("font-size"), FONT_SIZE);
        s_tags->defineInitialSymbol(_CS("font-style"), FONT_STYLE);
        s_tags->defineInitialSymbol(_CS("font-weight"), FONT_WEIGHT);
        s_tags->defineInitialSymbol(_CS("text-align"), TEXT_ALIGN);
        s_tags->defineInitialSymbol(_CS("border-bottom"), BORDER_BOTTOM);
        s_tags->defineInitialSymbol(_CS("border-bottom-width"), BORDER_BOTTOM_WIDTH);
        s_tags->defineInitialSymbol(_CS("border-top"), BORDER_TOP);
        s_tags->defineInitialSymbol(_CS("padding"), PADDING);
        s_tags->defineInitialSymbol(_CS("padding-top"), PADDING_TOP);
        s_tags->defineInitialSymbol(_CS("padding-left"), PADDING_LEFT);
        s_tags->defineInitialSymbol(_CS("padding-bottom"), PADDING_BOTTOM);
        s_tags->defineInitialSymbol(_CS("padding-right"), PADDING_RIGHT);
        s_tags->defineInitialSymbol(_CS("border"), BORDER);
        s_tags->defineInitialSymbol(_CS("background-image"), BACKGROUND_IMAGE);
        s_tags->defineInitialSymbol(_CS("display"), DISPLAY);
        s_tags->defineInitialSymbol(_CS("float"), FLOAT);
        s_tags->defineInitialSymbol(_CS("text-align"), TEXT_ALIGN);
        s_tags->defineInitialSymbol(_CS("width"), WIDTH);
        s_tags->defineInitialSymbol(_CS("height"), HEIGHT);
        s_tags->defineInitialSymbol(_CS("scale"), SCALE);
        s_tags->defineInitialSymbol(_CS("z-index"), Z_INDEX);
        s_tags->defineInitialSymbol(_CS("focusable"), FOCUSABLE);
        s_tags->defineInitialSymbol(_CS("flex-direction"), FLEX_DIRECTION);
    }

    return s_tags;
}

void StyleTags::defineInitialSymbol(const String& cssText, LInt cssType)
{
    // Use Tagmap accelerate search speed
    m_map.put(HashString(cssText), cssType);
}

void StyleTags::destroyInstance()
{
    if (s_tags) {
        delete s_tags;
    }
}

LInt StyleTags::symbolAsInt(const String& key)
{
    return m_map.get(HashString(key, LFalse));
}
}
