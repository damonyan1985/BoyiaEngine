#include "JSViewGroup.h"
#include "TextView.h"
#include "UIView.h"
#include "SalLog.h"
#include "BlockView.h"
#include "UIOperation.h"

namespace mjs
{
JSViewGroup::JSViewGroup(yanbo::HtmlView* item)
{
	m_item = item;
}

JSViewGroup::JSViewGroup(const String& id, LBool selectable)
{
	m_item = new yanbo::BlockView(id, selectable);
}

LVoid JSViewGroup::setText(const String& text)
{
//    if (m_item->m_children.count()) {
//    	yanbo::HtmlView* firstChild = *m_item->m_children.begin();
//    	if (firstChild && firstChild->isText()) {
//			yanbo::TextView* item = (yanbo::TextView*) firstChild;
//			item->setText(text);
//    	}
//    } else {
//    	yanbo::TextView* item = new yanbo::TextView(_CS(""), text, LFalse);
//    	item->setParent(m_item);
//    	m_item->addChild(item);
//
//    	yanbo::ResourceLoader* loader = yanbo::UIView::getInstance()->getLoader();
//    	m_item->setStyle(loader->render()->getCssManager(), NULL);
//    }

    yanbo::UIOperation::instance()->opSetText(m_item, text);
}

void JSViewGroup::appendView(JSView* view)
{
	//KLOG("JSViewDoc::appendDocument0");
    //HtmlView* item = m_doc->getItemByID(id);
    //KLOG("JSViewDoc::appendDocument");
    if (view && view->item())
    {
    	KFORMATLOG("JSViewDoc::appendDocument begin %s", (const char*)m_item->getTagName().GetBuffer());
    	//view->item()->setParent(m_item);
    	//m_item->addChild(view->item());
    	yanbo::UIOperation::instance()->opAddChild(m_item, view->item());
    }
}
}
