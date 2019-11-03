#include "EditorAndroid.h"
#include "AutoObject.h"
#include "JNIUtil.h"

namespace util {

static EditorAndroid* s_instance = NULL;
EditorAndroid::EditorAndroid()
    : m_view(NULL)
{
}

LVoid EditorAndroid::showKeyboard(const String& text)
{
    jstring strText = strToJstring(yanbo::JNIUtil::getEnv(), GET_STR(text));
    JNIUtil::callStaticVoidMethod(
        "com/boyia/app/core/BoyiaUIView",
        "showKeyboard",
        "(JLjava/lang/String;)V",
        (jlong)m_view, strText);
}

LVoid EditorAndroid::setView(LVoid* view)
{
    m_view = view;
}

Editor* Editor::get(LVoid* view)
{
    if (NULL == s_instance) {
        s_instance = new EditorAndroid();
    }

    s_instance->setView(view);
    return s_instance;
}
}
