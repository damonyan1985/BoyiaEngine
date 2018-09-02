#ifndef HtmlTags_h
#define HtmlTags_h

#include "TagMap.h"
#include "UtilString.h"

namespace yanbo
{
class HtmlTags
{
public:
	typedef enum HtmlType
	{
		TAGNONE = 0,
		HTML,
		HEAD,
		META,
		BR,
		UL,
		DIV,
		TR,
		TD,
		LI,
		P,
		H1,
		H2,
		H3,
		H4,
		STRONG,
		B,
		A,
		PRE,
		CENTER,
		FONT,
		IMG,
		I,
		EM,
		STYLE,
		SCRIPT,
		FORM,
		INPUT,
		SELECT,
		BUTTON,
		TEXTAREA,
		BODY,
		VIEW,
		TABLE,
		TITLE,
		OBJECT,
		OPTION,
		SPAN,
		LINK,
		BASE,
		GAME,
		VIDEO,
		TAGEND
	} HtmlType;
	
public:
	static HtmlTags* getInstance();
	static void destroyInstance();
	
	void defineInitialSymbol(const String& htmlText, LInt htmlType);
	~HtmlTags();
	
	int symbolAsInt(String& key);
	void clear();
	
private:
	HtmlTags();
	
private:
	static HtmlTags* m_htmlTags;
	TagMap m_map;
};

}
#endif
