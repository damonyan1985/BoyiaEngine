#ifndef EditorIOS_h
#define EditorIOS_h

#include "LGdi.h"

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
}

Editor* Editor::get()
{
    static EditorIOS sInstance;
    return &sInstance;
}
}

#endif
