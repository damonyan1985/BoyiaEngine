/*
============================================================================
 Name        : JSViewDoc.h
 Author      : yanbo
 Version     : JSViewDoc v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-2-5
 Description : support Original
============================================================================
*/
#ifndef JSViewDoc_h
#define JSViewDoc_h

#include "HtmlDocument.h"
#include "Document.h"
#include "JSView.h"

namespace mjs
{
class JSViewDoc : public JSView
{
public:
	JSViewDoc();
	virtual ~JSViewDoc();

	void loadHTML(const String& url);
	void setDocument(yanbo::HtmlDocument* doc);
	void removeDocument(String& id, JSViewDoc* doc);

	yanbo::HtmlDocument* getDocument() const;

	JSView* getItemByID(const String& id) const;

private:
	void fetchStream(const String& url, String& stream);

	static TagMap m_domMap;
	yanbo::HtmlDocument* m_doc;
};
}

#endif
