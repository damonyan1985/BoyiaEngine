#ifndef JSBase_h
#define JSBase_h

#include "IViewListener.h"
#include "KVector.h"
#include "BoyiaCore.h"

namespace boyia
{
#define EVENT_MAX_SIZE 10
class JSBase : public yanbo::IViewListener
{
public:
	JSBase();
	virtual ~JSBase();

    // HTMLView事件监听
    virtual void onPressDown(void* view);
    virtual void onPressMove(void* view);
    virtual void onPressUp(void* view);
    virtual void onKeyDown(int keyCode, void* view);
    virtual void onKeyUp(int keyCode, void* view);

    virtual void addListener(LInt type, BoyiaValue* callback);
    void setJSView(BoyiaValue* value);

protected:
	LInt m_type;
	BoyiaValue m_callbacks[EVENT_MAX_SIZE];
	BoyiaValue m_jsView;
};
}

#endif
