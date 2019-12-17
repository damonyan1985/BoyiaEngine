#ifndef BoyiaViewGroup_h
#define BoyiaViewGroup_h

#include "BoyiaView.h"
namespace boyia {
class BoyiaViewGroup : public BoyiaView {
public:
    BoyiaViewGroup(yanbo::HtmlView* item);
    BoyiaViewGroup(const String& id, LBool selectable);
    LVoid setText(const String& text);
    LVoid appendView(BoyiaView* view);
};
}

#endif
