/*
============================================================================
 Name        : BoyiaViewDoc.h
 Author      : yanbo
 Version     : BoyiaViewDoc v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-2-5
 Description : support Original
============================================================================
*/
#ifndef BoyiaViewDoc_h
#define BoyiaViewDoc_h

#include "HtmlDocument.h"
#include "BoyiaView.h"

namespace boyia
{
class BoyiaViewDoc : public BoyiaView
{
public:
	BoyiaViewDoc();
	virtual ~BoyiaViewDoc();

	void loadHTML(const String& url);
	void setDocument(yanbo::HtmlDocument* doc);
	void removeDocument(String& id, BoyiaViewDoc* doc);

	yanbo::HtmlDocument* getDocument() const;

	BoyiaView* getItemByID(const String& id) const;

private:
	void fetchStream(const String& url, String& stream);

	static TagMap m_domMap;
	yanbo::HtmlDocument* m_doc;
};
}

#endif
