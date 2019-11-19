#include "BoyiaViewDoc.h"
#include "AppManager.h"
#include "BoyiaImageView.h"
#include "BoyiaInputView.h"
#include "BoyiaViewGroup.h"
#include "DOMBuilder.h"
#include "StringUtils.h"
#include "UIOperation.h"
#include "UIView.h"

namespace boyia {
using namespace yanbo;

IdentityMap BoyiaViewDoc::m_domMap(20);

BoyiaViewDoc::BoyiaViewDoc()
    : m_doc(NULL)
{
}

BoyiaViewDoc::~BoyiaViewDoc()
{
    // 不允许删除rootdoc
    if (m_doc && m_doc != UIView::getInstance()->getDocument()) {
        delete m_doc;
    }
}

void BoyiaViewDoc::loadHTML(const String& url)
{
    KFORMATLOG("BoyiaViewDoc::loadHTML begin %d", 1);
    m_doc = new HtmlDocument();

    // 获取文本
    String stream;

    LUint key = StringUtils::hashCode(url);
    DOMBuilder* dom = (DOMBuilder*)m_domMap.get(key);
    //LInt domPtr = m_domMap.get(url);
    if (!dom) {
        KFORMATLOG("BoyiaViewDoc::loadHTML first create DOM %d", 1);
        dom = new DOMBuilder();
        m_domMap.put(key, (LIntPtr)dom);
        m_domMap.sort();

        AppManager::instance()->network()->syncLoadUrl(url, stream);
        //fetchStream(url, stream);
        KFORMATLOG("BoyiaViewDoc::loadHTML string=%s", stream.GetBuffer());
    }

    //dom->createDocument(stream, m_doc, NULL);
    dom->with(m_doc).build(stream);
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
    KFORMATLOG("BoyiaViewDoc::getItemByID id=%s", GET_STR(id));
    HtmlView* item = m_doc->getItemByID(id);
    KLOG("BoyiaViewDoc::getItemByID end");
    switch (item->getTagType()) {
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
