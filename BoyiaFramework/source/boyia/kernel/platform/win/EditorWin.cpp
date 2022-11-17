#ifndef EditorWin_h
#define EditorWin_h

#include "LGdi.h"

namespace util {
class EditorWin : public Editor {
public:
    EditorWin();
    virtual LVoid showKeyboard(const String& text, LInt cursor);
};

EditorWin::EditorWin()
{
}

LVoid EditorWin::showKeyboard(const String& text, LInt cursor)
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
