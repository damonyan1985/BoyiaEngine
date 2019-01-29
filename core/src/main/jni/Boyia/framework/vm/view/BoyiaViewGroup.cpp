#include "BoyiaViewGroup.h"
#include "TextView.h"
#include "UIView.h"
#include "SalLog.h"
#include "BlockView.h"
#include "UIOperation.h"

namespace boyia
{
BoyiaViewGroup::BoyiaViewGroup(yanbo::HtmlView* item)
{
	m_item = item;
}

BoyiaViewGroup::BoyiaViewGroup(const String& id, LBool selectable)
{
	m_item = new yanbo::BlockView(id, selectable);
}

LVoid BoyiaViewGroup::setText(const String& text)
{
    yanbo::UIOperation::instance()->opSetText(m_item, text);
}

void BoyiaViewGroup::appendView(BoyiaView* view)
{
    if (view && view->item())
    {
    	KFORMATLOG("BoyiaViewDoc::appendDocument begin %s", (const char*)m_item->getTagName().GetBuffer());
    	yanbo::UIOperation::instance()->opAddChild(m_item, view->item());
    }
}
}
