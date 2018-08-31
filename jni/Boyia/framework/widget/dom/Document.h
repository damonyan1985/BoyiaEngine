#ifndef Document_h
#define Document_h

#include "UtilString.h"
#include "HtmlDocument.h"
#include "CssParser.h"
#include "IViewListener.h"

namespace yanbo
{
class Document
{
public:
	enum DocType
	{
        ETiny2Doc,
        EExpatDoc,
	};
	static Document* create(DocType type);

public:
	virtual ~Document() {};
	virtual void createDocument(
			const String& buffer,
			HtmlDocument* doc,
			util::CssParser* cssParser) = 0;
};
}
#endif
