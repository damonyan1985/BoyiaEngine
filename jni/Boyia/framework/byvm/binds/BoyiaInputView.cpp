#include "BoyiaInputView.h"
#include "InputView.h"

namespace boyia {
BoyiaInputView::BoyiaInputView(yanbo::HtmlView* item) {
	m_item = item;
	//m_item->setListener(this);
}

void BoyiaInputView::setText(const String& text) {
	yanbo::InputView* item = static_cast<yanbo::InputView*>(m_item);
	item->setInputValue(text);
}
}
