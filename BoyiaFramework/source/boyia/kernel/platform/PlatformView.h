// For platform custom view

#ifndef PlatformView_h
#define PlatformView_h

#include "BlockView.h"
#include "TextureCache.h"

namespace yanbo {
class ViewInfo;
class PlatformView : public BlockView {
public:
    PlatformView(
        const String& id,
        LBool selectable,
        const String& viewType);

    ~PlatformView();

    virtual void layout();
    virtual void paint(LGraphicsContext& gc);

    Texture* texture();

private:
    ViewInfo* m_viewInfo;
};
}

#endif