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
#include "WeakPtr.h"

namespace boyia {
#define EVENT_MAX_SIZE 10

class BoyiaView : public BoyiaBase, public yanbo::IViewListener {
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

    // HTMLView事件监听
    virtual LVoid onPressDown(LVoid* view);
    virtual LVoid onPressMove(LVoid* view);
    virtual LVoid onPressUp(LVoid* view);
    virtual LVoid onKeyDown(LInt keyCode, LVoid* view);
    virtual LVoid onKeyUp(LInt keyCode, LVoid* view);

    virtual LVoid addListener(LInt type, BoyiaValue* callback);
    LVoid setBoyiaView(BoyiaValue* value);
    LVoid setVisible(bool visible);

protected:
    LInt m_type;
    BoyiaValue m_callbacks[EVENT_MAX_SIZE];
    BoyiaValue m_boyiaView;

    WeakPtr<yanbo::HtmlView> m_item;
};
}

#endif
