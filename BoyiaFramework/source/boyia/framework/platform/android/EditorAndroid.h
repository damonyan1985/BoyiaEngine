#ifndef AndroidEditor_h
#define AndroidEditor_h

#include "LGdi.h"

namespace util {

class EditorAndroid : public Editor {
public:
    EditorAndroid();
    virtual LVoid showKeyboard(const String& text);
    LVoid setView(LVoid* view);

private:
    LVoid* m_view;
};
}

#endif
