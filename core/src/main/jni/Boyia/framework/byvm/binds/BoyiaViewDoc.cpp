#include "BoyiaViewDoc.h"
#include "JNIUtil.h"
#include "AutoObject.h"
#include "UIView.h"
#include "StringUtils.h"
#include "BoyiaImageView.h"
#include "BoyiaViewGroup.h"
#include "BoyiaInputView.h"
#include "UIOperation.h"
#include "DOMBuilder.h"

namespace boyia
{
using namespace yanbo;

TagMap BoyiaViewDoc::m_domMap(20);

BoyiaViewDoc::BoyiaViewDoc()
    : m_doc(NULL)
{
}

BoyiaViewDoc::~BoyiaViewDoc()
{
	// 不允许删除rootdoc
	if (m_doc && m_doc != UIView::getInstance()->getDocument())
	{
		delete m_doc;
	}
}

void BoyiaViewDoc::fetchStream(const String& url, String& stream)
{
	JNIUtil::loadHTML(url, stream);
}

void BoyiaViewDoc::loadHTML(const String& url)
{
	KFORMATLOG("BoyiaViewDoc::loadHTML begin %d", 1);
	m_doc = new HtmlDocument();

	// 获取文本
	String stream;

	LUint key = StringUtils::hashCode(url);
	DOMBuilder* dom = (DOMBuilder*) m_domMap.get(key);
	//LInt domPtr = m_domMap.get(url);
    if (!dom)
    {
    	KFORMATLOG("BoyiaViewDoc::loadHTML first create DOM %d", 1);
        dom = new DOMBuilder();
    	m_domMap.put(key, (LIntPtr) dom);

    	fetchStream(url, stream);
    	KFORMATLOG("BoyiaViewDoc::loadHTML string=%s", stream.GetBuffer());
    }

    dom->createDocument(stream, m_doc, NULL);
	m_item = m_doc->getRenderTreeRoot();

//	ResourceLoader* loader = UIView::getInstance()->getLoader();
//	m_item->setStyle(loader->render()->getCssManager(), NULL);

    yanbo::UIOperation::instance()->opApplyDomStyle(m_item);
}

void BoyiaViewDoc::setDocument(yanbo::HtmlDocument* doc)
{
	m_doc = doc;
	m_item = m_doc->getRenderTreeRoot();
}

void BoyiaViewDoc::removeDocument(String& id, BoyiaViewDoc* doc)
{
}

yanbo::HtmlDocument* BoyiaViewDoc::getDocument() const
{
	return m_doc;
}

BoyiaView* BoyiaViewDoc::getItemByID(const String& id) const
{
	HtmlView* item = m_doc->getItemByID(id);
	switch (item->getTagType())
	{
	case HtmlTags::IMG:
		return new BoyiaImageView(item);
	case HtmlTags::DIV:
		return new BoyiaViewGroup(item);
	case HtmlTags::INPUT:
		return new BoyiaInputView(item);
	}

	return NULL;
}

}
