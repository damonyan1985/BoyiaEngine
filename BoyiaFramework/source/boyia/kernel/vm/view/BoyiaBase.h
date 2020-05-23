#ifndef BoyiaBase_h
#define BoyiaBase_h

#include "BoyiaCore.h"
#include "IViewListener.h"
#include "KVector.h"
#include "BoyiaRuntime.h"

namespace boyia {
#define EVENT_MAX_SIZE 10
class BoyiaBase : public yanbo::IViewListener {
public:
    BoyiaBase(BoyiaRuntime* runtime);
    virtual ~BoyiaBase();

    // HTMLView事件监听
    virtual LVoid onPressDown(LVoid* view);
    virtual LVoid onPressMove(LVoid* view);
    virtual LVoid onPressUp(LVoid* view);
    virtual LVoid onKeyDown(LInt keyCode, LVoid* view);
    virtual LVoid onKeyUp(LInt keyCode, LVoid* view);

    virtual LVoid addListener(LInt type, BoyiaValue* callback);
    LVoid setBoyiaView(BoyiaValue* value);

    BoyiaRuntime* runtime() const;

protected:
    LInt m_type;
    BoyiaValue m_callbacks[EVENT_MAX_SIZE];
    BoyiaValue m_boyiaView;
    BoyiaRuntime* m_runtime;
};
}

#endif
