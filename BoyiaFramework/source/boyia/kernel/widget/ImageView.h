/*
 * ImageView.h
 *
 *  Created on: 2011-6-30
 *      Author: yanbo
 */

#ifndef ImageView_h
#define ImageView_h

#include "InlineView.h"
#include "OwnerPtr.h"

namespace yanbo {

class ImageView : public InlineView {
public:
    ImageView(
        const String& id,
        LBool selectable,
        const String& src);

protected:
    ~ImageView();

public:
    virtual LBool isImage() const;
    const String& url() const;
    LVoid setUrl(const String& url);

    virtual LVoid paint(LGraphicsContext& gc);
    virtual LVoid layoutInline(RenderContext& rc);

private:
    OwnerPtr<LImage> m_image;
    String m_src;
};
}

#endif /* ImageView_H_ */
