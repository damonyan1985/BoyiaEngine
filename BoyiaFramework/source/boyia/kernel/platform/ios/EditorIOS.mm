#ifndef EditorIOS_h
#define EditorIOS_h

#include "LGdi.h"
// oc import
#import "IOSRenderer.h"

namespace util {
// TODO
class EditorIOS : public Editor {
public:
    EditorIOS();
    virtual LVoid showKeyboard(const String& text, LInt cursor);
};

EditorIOS::EditorIOS()
{
}

LVoid EditorIOS::showKeyboard(const String& text, LInt cursor)
{
    Editor* editor = static_cast<Editor*>(this);
    IOSRenderer* renderer = [IOSRenderer renderer];
    if (!renderer) {
        return;
    }
    
    [renderer showKeyboard:STR_TO_OCSTR(text) cursor:cursor];
}

Editor* Editor::get()
{
    static EditorIOS sInstance;
    return &sInstance;
}
}

#endif
