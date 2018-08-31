#ifndef Tiny2Dom_h
#define Tiny2Dom_h

#include "Document.h"
#include "tinyxml2.h"
#include "HtmlView.h"

using namespace tinyxml2;

namespace yanbo
{
class Tiny2Dom : public Document
{
public:
	Tiny2Dom();
	~Tiny2Dom();

	virtual void createDocument(
			const String& buffer,
			HtmlDocument* doc,
			util::CssParser* cssParser);

private:
	HtmlView* createHtmlView(XMLNode* node, XMLNode* parentElem, HtmlView* parent);
	void createRenderTree(XMLNode* elem, XMLNode* parentElem, HtmlView* parent);

private:
	HtmlDocument*      m_htmlDoc;
	util::CssParser*   m_cssParser;
	XMLDocument*       m_xmlDoc;
};
}

#endif
