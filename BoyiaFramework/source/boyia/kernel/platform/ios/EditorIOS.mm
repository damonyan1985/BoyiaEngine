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
    virtual LVoid showKeyboard(const String& text);
};

EditorIOS::EditorIOS()
{
}

LVoid EditorIOS::showKeyboard(const String& text)
{
    Editor* editor = static_cast<Editor*>(this);
    IOSRenderer* renderer = [IOSRenderer renderer];
    if (!renderer) {
        return;
    }
    
    [renderer showKeyboard:STR_TO_OCSTR(text)];
}

Editor* Editor::get()
{
    static EditorIOS sInstance;
    return &sInstance;
}
}

#endif
