/*
 * HtmlTags.cpp
 *
 *  Created on: 2011-6-22
 *      Author: yanbo
 */
#include "HtmlTags.h"
#include "StringUtils.h"

namespace yanbo
{
HtmlTags* HtmlTags::m_htmlTags = NULL;

HtmlTags::HtmlTags()
    : m_map(TAGEND)
{	
}

HtmlTags::~HtmlTags()
{
}

void HtmlTags::defineInitialSymbol(const String& htmlText, LInt htmlType)
{
	m_map.put(htmlText, htmlType);
}

HtmlTags* HtmlTags::getInstance()
{
	if (m_htmlTags == NULL)
	{
		m_htmlTags = new HtmlTags();	
		m_htmlTags->defineInitialSymbol(_CS("UNDEFINED"), TAGNONE); // 0
		/*
		 * Initializing HTML tags.
		 */
		m_htmlTags->defineInitialSymbol(_CS("html"), HTML);
		m_htmlTags->defineInitialSymbol(_CS("head"), HEAD); // 1
		m_htmlTags->defineInitialSymbol(_CS("meta"), META); // 2
		m_htmlTags->defineInitialSymbol(_CS("br"), BR); // 3
		m_htmlTags->defineInitialSymbol(_CS("ul"), UL); // 4
		m_htmlTags->defineInitialSymbol(_CS("div"), DIV); // 5
		m_htmlTags->defineInitialSymbol(_CS("tr"), TR); // 6
		m_htmlTags->defineInitialSymbol(_CS("td"), TD); // 7
		m_htmlTags->defineInitialSymbol(_CS("li"), LI); // 8
		m_htmlTags->defineInitialSymbol(_CS("p"), P); // 9
		m_htmlTags->defineInitialSymbol(_CS("h1"), H1); // 10
		m_htmlTags->defineInitialSymbol(_CS("h2"), H2); // 11
		m_htmlTags->defineInitialSymbol(_CS("h3"), H3); // 12
		m_htmlTags->defineInitialSymbol(_CS("h4"), H4); // 13
		m_htmlTags->defineInitialSymbol(_CS("strong"), STRONG); // 14
		m_htmlTags->defineInitialSymbol(_CS("b"), B); // 15
		m_htmlTags->defineInitialSymbol(_CS("a"), A); // 16
		m_htmlTags->defineInitialSymbol(_CS("pre"), PRE); // 17
		m_htmlTags->defineInitialSymbol(_CS("center"), CENTER); // 18
		m_htmlTags->defineInitialSymbol(_CS("font"), FONT); // 19
		m_htmlTags->defineInitialSymbol(_CS("img"), IMG); // 20
		m_htmlTags->defineInitialSymbol(_CS("i"), I); // 21
		m_htmlTags->defineInitialSymbol(_CS("em"), EM); // 22
		m_htmlTags->defineInitialSymbol(_CS("style"), STYLE); // 23
		m_htmlTags->defineInitialSymbol(_CS("script"), SCRIPT); // 24
		m_htmlTags->defineInitialSymbol(_CS("form"), FORM); // 25
		m_htmlTags->defineInitialSymbol(_CS("input"), INPUT); // 26
		m_htmlTags->defineInitialSymbol(_CS("select"), SELECT); // 27
		m_htmlTags->defineInitialSymbol(_CS("button"), BUTTON); // 28
		m_htmlTags->defineInitialSymbol(_CS("textarea"), TEXTAREA); // 30
		m_htmlTags->defineInitialSymbol(_CS("body"), BODY); // 32
		m_htmlTags->defineInitialSymbol(_CS("view"), VIEW); // 32
		m_htmlTags->defineInitialSymbol(_CS("table"), TABLE); // 33
		m_htmlTags->defineInitialSymbol(_CS("title"), TITLE); // 35
		m_htmlTags->defineInitialSymbol(_CS("object"), OBJECT); //36
		m_htmlTags->defineInitialSymbol(_CS("option"), OPTION); 
		m_htmlTags->defineInitialSymbol(_CS("span"), SPAN); 
		m_htmlTags->defineInitialSymbol(_CS("link"), LINK);
		m_htmlTags->defineInitialSymbol(_CS("base"), BASE);
		m_htmlTags->defineInitialSymbol(_CS("video"), VIDEO);

		m_htmlTags->m_map.sort();
	}

	return m_htmlTags;
}

void HtmlTags::clear()
{
	m_map.clear();
}

void HtmlTags::destroyInstance()
{
	if (m_htmlTags)
	{
		delete m_htmlTags;
		m_htmlTags = NULL;
	}

}

int HtmlTags::symbolAsInt(String& key)
{
	//LUint hash = StringUtils::hashCode(inSymbol.ToLower());
	return m_map.get(key.ToLower());
}

}
