#include "BoyiaViewGroup.h"
#include "BlockView.h"
#include "SalLog.h"
#include "TextView.h"
#include "UIOperation.h"
#include "UIView.h"

namespace boyia {
BoyiaViewGroup::BoyiaViewGroup(BoyiaRuntime* runtime, yanbo::HtmlView* item)
    : BoyiaView(runtime)
{
    m_item = item;
}

BoyiaViewGroup::BoyiaViewGroup(BoyiaRuntime* runtime, const String& id, LBool selectable)
    : BoyiaView(runtime)
{
    m_item = new yanbo::BlockView(id, selectable);
}

LVoid BoyiaViewGroup::setText(const String& text)
{
    yanbo::UIOperation::instance()->opSetText(m_item, text);
}

LVoid BoyiaViewGroup::appendView(BoyiaView* view)
{
    if (view && view->item()) {
        KFORMATLOG("BoyiaViewDoc::appendDocument begin %s", (const char*)m_item->getTagName().GetBuffer());
        yanbo::UIOperation::instance()->opAddChild(m_item, view->item());
    }
}
}
