#include "BoyiaInputView.h"
#include "InputView.h"
#include "UIOperation.h"

namespace boyia {
BoyiaInputView::BoyiaInputView(BoyiaRuntime* runtime, yanbo::HtmlView* item)
    : BoyiaView(runtime)
{
    m_item = item;
}

LVoid BoyiaInputView::setText(const String& text)
{
    if (m_item) {
        runtime()->view()->operation()->opSetInput(m_item, text);
    }
}
}
