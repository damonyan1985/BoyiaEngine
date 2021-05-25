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

#include "BoyiaView.h"
#include "HtmlDocument.h"

namespace yanbo {
class DOMBuilder;
}
namespace boyia {
class BoyiaViewDoc : public BoyiaView {
public:
    BoyiaViewDoc(BoyiaRuntime* runtime);
    virtual ~BoyiaViewDoc();

    LVoid loadHTML(const String& url);
    LVoid setDocument(yanbo::HtmlDocument* doc);
    LVoid removeDocument(String& id, BoyiaViewDoc* doc);

    yanbo::HtmlDocument* getDocument() const;

    BoyiaView* getItemByID(const String& id) const;

private:
    yanbo::HtmlDocument* m_doc;
};
}

#endif
