#include "EditorAndroid.h"
#include "JNIUtil.h"
namespace util
{
static EditorAndroid* s_instance = NULL;
EditorAndroid::EditorAndroid()
    : m_view(NULL)
{
}

LVoid EditorAndroid::showKeyboard()
{
	JNIUtil::callStaticVoidMethod("com/boyia/app/core/BoyiaUIView", "showKeyboard", "(J)V", (jlong)m_view);
}

LVoid EditorAndroid::setView(LVoid* view)
{
	m_view = view;
}

Editor* Editor::get(LVoid* view)
{
    if (NULL == s_instance)
    {
    	s_instance = new EditorAndroid();
    }

    s_instance->setView(view);
}
}
