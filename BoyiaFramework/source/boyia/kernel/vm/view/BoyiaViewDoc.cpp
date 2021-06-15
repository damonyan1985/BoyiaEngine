#include "BoyiaViewDoc.h"
#include "AppManager.h"
#include "BoyiaImageView.h"
#include "BoyiaInputView.h"
#include "BoyiaViewGroup.h"
#include "StringUtils.h"
#include "UIOperation.h"
#include "UIView.h"

namespace boyia {
using namespace yanbo;
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
    m_doc->setView(runtime()->view());

    // 获取文本
    String stream;

    DOMBuilder* dom = runtime()->domMap()->get(HashString(url, LFalse));
    if (!dom) {
        KFORMATLOG("BoyiaViewDoc::loadHTML first create DOM %d", 1);
        dom = new DOMBuilder();
        runtime()->domMap()->put(url, dom);

        AppManager::instance()->network()->syncLoadUrl(url, stream);
        BOYIA_LOG("BoyiaViewDoc::loadHTML string=%s", stream.GetBuffer());
    }

    dom->with(m_doc).build(stream);
    m_item = m_doc->getRenderTreeRoot();

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
    if (!item) {
        BOYIA_LOG("The view with id:%s is null", GET_STR(id));
        return kBoyiaNull;
    }

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
