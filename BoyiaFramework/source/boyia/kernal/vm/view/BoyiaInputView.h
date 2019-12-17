#ifndef BoyiaInputView_h
#define BoyiaInputView_h

#include "BoyiaView.h"

namespace boyia {
class BoyiaInputView : public BoyiaView {
public:
    BoyiaInputView(yanbo::HtmlView* item);
    LVoid setText(const String& text);
};
}
#endif
