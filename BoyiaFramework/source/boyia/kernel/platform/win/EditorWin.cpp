#ifndef EditorWin_h
#define EditorWin_h

#include "LGdi.h"

namespace util {
class EditorWin : public Editor {
public:
    EditorWin();
    virtual LVoid showKeyboard(const String& text);
};

EditorWin::EditorWin()
{
}

LVoid EditorWin::showKeyboard(const String& text)
{
}

Editor* Editor::get()
{
    // static EditorWin sInstance;
    // return &sInstance;
    return new EditorWin();
}
}

#endif
