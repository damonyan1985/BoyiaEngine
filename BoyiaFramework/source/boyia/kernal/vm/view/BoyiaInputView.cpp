#include "BoyiaInputView.h"
#include "InputView.h"
#include "UIOperation.h"

namespace boyia {
BoyiaInputView::BoyiaInputView(yanbo::HtmlView* item)
{
    m_item = item;
}

LVoid BoyiaInputView::setText(const String& text)
{
    //yanbo::InputView* item = static_cast<yanbo::InputView*>(m_item);
    //item->setInputValue(text);
    yanbo::UIOperation::instance()->opSetInput(m_item, text);
}
}
