#include "EditorAndroid.h"
#include "AutoObject.h"
#include "JNIUtil.h"

namespace util {

EditorAndroid::EditorAndroid()
{
}

LVoid EditorAndroid::showKeyboard(const String& text)
{
    jstring strText = strToJstring(yanbo::JNIUtil::getEnv(), text);
    Editor* editor = static_cast<Editor*>(this);
    JNIUtil::callStaticVoidMethod(
        "com/boyia/app/core/BoyiaUIView",
        "showKeyboard",
        "(JLjava/lang/String;)V",
        (jlong)editor, strText);
}

Editor* Editor::get()
{
    static EditorAndroid sInstance;
    return &sInstance;
}
}
