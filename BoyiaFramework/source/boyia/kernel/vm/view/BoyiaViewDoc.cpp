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

HashMap<HashString, yanbo::DOMBuilder*> BoyiaViewDoc::s_domMap;

BoyiaViewDoc::BoyiaViewDoc(BoyiaRuntime* runtime)
    : BoyiaView(runtime)
    , m_doc(kBoyiaNull)
{
}

BoyiaViewDoc::~BoyiaViewDoc()
{
    // 不允许删除rootdoc
    if (m_doc && m_doc != m_runtime->view()->getDocument()) {
        delete m_doc;
    }
}

LVoid BoyiaViewDoc::loadHTML(const String& url)
{
    KFORMATLOG("BoyiaViewDoc::loadHTML begin %d", 1);
    m_doc = new HtmlDocument();

    // 获取文本
    String stream;

    //LUint key = StringUtils::hashCode(url);
    DOMBuilder* dom = s_domMap.get(url);
    //LInt domPtr = m_domMap.get(url);
    if (!dom) {
        KFORMATLOG("BoyiaViewDoc::loadHTML first create DOM %d", 1);
        dom = new DOMBuilder();
        s_domMap.put(url, dom);

        AppManager::instance()->network()->syncLoadUrl(url, stream);
        //fetchStream(url, stream);
        BOYIA_LOG("BoyiaViewDoc::loadHTML string=%s", stream.GetBuffer());
    }

    //dom->createDocument(stream, m_doc, NULL);
    dom->with(m_doc).build(stream);
    m_item = m_doc->getRenderTreeRoot();

    //	ResourceLoader* loader = UIView::getInstance()->getLoader();
    //	m_item->setStyle(loader->render()->getCssManager(), NULL);

    runtime()->view()->operation()->opApplyDomStyle(m_item);
}

LVoid BoyiaViewDoc::setDocument(yanbo::HtmlDocument* doc)
{
    m_doc = doc;
    m_item = m_doc->getRenderTreeRoot();
}

LVoid BoyiaViewDoc::removeDocument(String& id, BoyiaViewDoc* doc)
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
        return new BoyiaImageView(runtime(), item);
    case HtmlTags::DIV:
        return new BoyiaViewGroup(runtime(), item);
    case HtmlTags::INPUT:
        return new BoyiaInputView(runtime(), item);
    }

    return kBoyiaNull;
}
}
