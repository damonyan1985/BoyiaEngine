#include "JSInputView.h"
#include "InputView.h"

namespace mjs {
JSInputView::JSInputView(yanbo::HtmlView* item) {
	m_item = item;
	//m_item->setListener(this);
}

void JSInputView::setText(const String& text) {
	yanbo::InputView* item = static_cast<yanbo::InputView*>(m_item);
	item->setInputValue(text);
}
}
