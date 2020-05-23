#ifndef BoyiaImageView_h
#define BoyiaImageView_h

#include "BoyiaView.h"
namespace boyia {
class BoyiaImageView : public BoyiaView {
public:
    BoyiaImageView(BoyiaRuntime* runtime, yanbo::HtmlView* item);
    LVoid setImageUrl(const String& url);
    LVoid loadImage(const String& url);
};
}

#endif
