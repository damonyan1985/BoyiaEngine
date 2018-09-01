#include "JSViewDoc.h"
#include "JNIUtil.h"
#include "AutoObject.h"
#include "UIView.h"
#include "StringUtils.h"
#include "JSImageView.h"
#include "JSViewGroup.h"
#include "JSInputView.h"

namespace boyia
{
using namespace yanbo;

TagMap JSViewDoc::m_domMap(20);

JSViewDoc::JSViewDoc()
    : m_doc(NULL)
{
}

JSViewDoc::~JSViewDoc()
{
	// 不允许删除rootdoc
	if (m_doc && m_doc != UIView::getInstance()->getDocument())
	{
		delete m_doc;
	}
}

void JSViewDoc::fetchStream(const String& url, String& stream)
{
	JNIUtil::loadHTML(url, stream);
}

void JSViewDoc::loadHTML(const String& url)
{
	KFORMATLOG("JSViewDoc::loadHTML begin %d", 1);
	m_doc = new HtmlDocument();

	// 获取文本
	String stream;

	LUint key = StringUtils::hashCode(url);
	Document* dom = (Document*) m_domMap.get(key);
	//LInt domPtr = m_domMap.get(url);
    if (!dom)
    {
    	KFORMATLOG("JSViewDoc::loadHTML first create DOM %d", 1);
    	dom = Document::create(Document::ETiny2Doc);
    	m_domMap.put(key, (LInt) dom);

    	fetchStream(url, stream);
    	KFORMATLOG("JSViewDoc::loadHTML string=%s", stream.GetBuffer());
    }

    dom->createDocument(stream, m_doc, NULL);
	m_item = m_doc->getRenderTreeRoot();
	ResourceLoader* loader = UIView::getInstance()->getLoader();
	m_item->setStyle(loader->render()->getCssManager(), NULL);
}

void JSViewDoc::setDocument(yanbo::HtmlDocument* doc)
{
	m_doc = doc;
	m_item = m_doc->getRenderTreeRoot();
}

void JSViewDoc::removeDocument(String& id, JSViewDoc* doc)
{
}

yanbo::HtmlDocument* JSViewDoc::getDocument() const
{
	return m_doc;
}

JSView* JSViewDoc::getItemByID(const String& id) const
{
	HtmlView* item = m_doc->getItemByID(id);
	switch (item->getTagType())
	{
	case HtmlTags::IMG:
		return new JSImageView(item);
	case HtmlTags::DIV:
		return new JSViewGroup(item);
	case HtmlTags::INPUT:
		return new JSInputView(item);
	}

	return NULL;
}

}
