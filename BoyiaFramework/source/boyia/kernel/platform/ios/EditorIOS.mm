#ifndef EditorIOS_h
#define EditorIOS_h

#include "LGdi.h"
#include "RenderEngineIOS.h"
#include "RenderThread.h"

namespace util {
// TODO
class EditorIOS : public Editor {
public:
    EditorIOS();
    virtual LVoid showKeyboard(const String& text);
};

EditorIOS::EditorIOS()
{
}

LVoid EditorIOS::showKeyboard(const String& text)
{
    yanbo::RenderEngineIOS* engine = static_cast<yanbo::RenderEngineIOS*>(yanbo::RenderThread::instance()->getRenderer());
    engine->showKeyboard();
}

Editor* Editor::get()
{
    static EditorIOS sInstance;
    return &sInstance;
}
}

#endif
