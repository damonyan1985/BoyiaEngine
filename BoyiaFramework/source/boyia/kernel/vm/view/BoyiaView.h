/*
============================================================================
 Name        : BoyiaView.h
 Author      : yanbo
 Version     : BoyiaView v1.0
 Copyright   : All Copyright Reserved
 Date        : 2018-2-5
 Description : support Original
============================================================================
*/
#ifndef BoyiaView_h
#define BoyiaView_h

#include "BoyiaBase.h"
#include "HtmlView.h"
#include "KVector.h"

namespace boyia {
class BoyiaView : public BoyiaBase {
public:
    BoyiaView(BoyiaRuntime* runtime);
    virtual ~BoyiaView();
    LVoid setX(LInt x);
    LVoid setY(LInt y);

    LInt left() const;
    LInt top() const;

    LInt width() const;
    LInt height() const;

    LVoid setStyle(String& cls);

    yanbo::HtmlView* item() const;

    LVoid drawView();
    LVoid commit();
    // 动画
    LVoid startScale(LInt scale, LInt duration);
    LVoid startOpacity(LInt opacity, LInt duration);
    LVoid startTranslate(const LPoint& point, LInt duration);

    virtual LVoid addListener(LInt type, BoyiaValue* callback);

protected:
    yanbo::HtmlView* m_item;
};
}

#endif
