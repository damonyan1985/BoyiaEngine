#include "FontAndroid.h"
#include "AutoObject.h"
#include "SalLog.h"
#include "StringUtils.h"
#include "LGdi.h"
#include "JNIUtil.h"

namespace util
{
struct JFontAndroid
{
	jweak     m_obj;
    jmethodID m_getFontWidth;
    jmethodID m_getFontHeight;
    jmethodID m_getTextWidth;
    jmethodID m_getLineSize;
    jmethodID m_getLineWidth;
    jmethodID m_getLineText;
    jmethodID m_calcTextLine;
    AutoJObject object(JNIEnv* env) {
        return getRealObject(env, m_obj);
    }
};

FontAndroid::FontAndroid()
    : m_privateFont(NULL)
{
	KLOG("FontAndroid::FontAndroid()");
	//JNIEnv* env = getJNIEnv();
	JNIEnv* env = yanbo::JNIUtil::getEnv();

	jclass clazz = yanbo::JNIUtil::getJavaClassID("com/boyia/app/core/BoyiaFont");
    jmethodID constructMethod = env->GetMethodID(clazz, "<init>",
                                           "()V");
    jobject obj = env->NewObject(clazz, constructMethod);
    init(env, clazz,  obj);
}

void FontAndroid::init(JNIEnv* env, jclass clazz, jobject obj)
{
	KLOG("FontAndroid::init");
    if (!m_privateFont)
    {
    	m_privateFont = new JFontAndroid;
    }
	KLOG("FontAndroid::init3");
	m_privateFont->m_obj = env->NewGlobalRef(obj);
	KLOG("FontAndroid::init4");
	m_privateFont->m_getFontWidth = GetJMethod(env, clazz, "getFontWidth", "(Ljava/lang/String;I)I");
	m_privateFont->m_getFontHeight = GetJMethod(env, clazz, "getFontHeight", "(I)I");
	m_privateFont->m_getTextWidth = GetJMethod(env, clazz, "getTextWidth", "(Ljava/lang/String;I)I");
    m_privateFont->m_getLineSize = GetJMethod(env, clazz, "getLineSize", "()I");
    m_privateFont->m_getLineWidth = GetJMethod(env, clazz, "getLineWidth", "(I)I");
    m_privateFont->m_getLineText = GetJMethod(env, clazz, "getLineText", "(I)Ljava/lang/String;");
    m_privateFont->m_calcTextLine = GetJMethod(env, clazz, "calcTextLine", "(Ljava/lang/String;II)I");

    env->DeleteLocalRef(obj);
    env->DeleteLocalRef(clazz);
}

FontAndroid::~FontAndroid()
{
	if (m_privateFont != NULL)
	{
		JNIEnv* env = yanbo::JNIUtil::getEnv();
        env->DeleteGlobalRef(m_privateFont->m_obj);
		delete m_privateFont;
	}

	m_privateFont = NULL;
}

LInt FontAndroid::getFontWidth(LUint8 ch, LInt size) const
{
	KLOG("FontAndroid::getFontWidth");;
    //StringW strW = chW;
    String astr = ch;
    //util::StringUtils::strWtoStr(chW, astr);

    //JNIEnv* env = m_privateFont->m_env;
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateFont->object(env);
    if (!javaObject.get())
        return 0;

    jstring strText = strToJstring(env, (const char*)astr.GetBuffer());
    LInt width = env->CallIntMethod(javaObject.get(), m_privateFont->m_getFontWidth,
    		strText, size);
    env->DeleteLocalRef(strText);
    return width;
}

LInt FontAndroid::getTextWidth(const String& text, LInt size) const
{
	KLOG("FontAndroid::getTextWidth");
    //String astr;
    //util::StringUtils::strWtoStr(text, astr);
    KLOG("FontAndroid::getTextWidth1");
    //JNIEnv* env = m_privateFont->m_env;

    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateFont->object(env);
    if (!javaObject.get())
    	return 0;

    KLOG("FontAndroid::getTextWidth2");
    jstring strText = strToJstring(env, (const char*)text.GetBuffer());
    LInt width = env->CallIntMethod(javaObject.get(), m_privateFont->m_getTextWidth,
    		strText, size);
    env->DeleteLocalRef(strText);
    return width;
}

LInt FontAndroid::getFontHeight(LInt size) const
{
	KLOG("FontAndroid::getFontHeight");

	if (m_privateFont == NULL)
	{
		KLOG("FontAndroid::getFontHeight0");
        return 0;
	}
    //JNIEnv* env = m_privateFont->m_env;

	JNIEnv* env = yanbo::JNIUtil::getEnv();
    KLOG("FontAndroid::getFontHeight1");
    AutoJObject javaObject = m_privateFont->object(env);
    KLOG("FontAndroid::getFontHeight2");
    if (!javaObject.get())
        return 0;

    KLOG("FontAndroid::getFontHeight1");
    return env->CallIntMethod(javaObject.get(), m_privateFont->m_getFontHeight, size);
}

LInt FontAndroid::getLineSize() const
{
    if (m_privateFont == NULL)
    {
        KLOG("FontAndroid::getFontHeight0");
        return 0;
    }
    //JNIEnv* env = m_privateFont->m_env;
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    KLOG("FontAndroid::getFontHeight1");
    AutoJObject javaObject = m_privateFont->object(env);
    KLOG("FontAndroid::getFontHeight2");
    if (!javaObject.get())
        return 0;

    KLOG("FontAndroid::getFontHeight1");
    return env->CallIntMethod(javaObject.get(), m_privateFont->m_getLineSize);
}

LInt FontAndroid::getLineWidth(LInt index) const
{
    if (m_privateFont == NULL)
    {
        KLOG("FontAndroid::getFontHeight0");
        return 0;
    }
    //JNIEnv* env = m_privateFont->m_env;
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    KLOG("FontAndroid::getFontHeight1");
    AutoJObject javaObject = m_privateFont->object(env);
    KLOG("FontAndroid::getFontHeight2");
    if (!javaObject.get())
        return 0;

    KLOG("FontAndroid::getFontHeight1");
    return env->CallIntMethod(javaObject.get(), m_privateFont->m_getLineWidth, index);
}

LVoid FontAndroid::getLineText(LInt index, String& text)
{
    if (m_privateFont == NULL)
    {
        KLOG("FontAndroid::getFontHeight0");
        return;
    }
    //JNIEnv* env = m_privateFont->m_env;

    JNIEnv* env = yanbo::JNIUtil::getEnv();
    KLOG("FontAndroid::getFontHeight1");
    AutoJObject javaObject = m_privateFont->object(env);
    KLOG("FontAndroid::getFontHeight2");
    if (!javaObject.get())
        return;

    KLOG("FontAndroid::getFontHeight1");
    jstring jstr = (jstring) env->CallObjectMethod(javaObject.get(), m_privateFont->m_getLineText, index);
    jstringTostr(env, jstr, text);
    env->DeleteLocalRef(jstr);
}

LInt FontAndroid::calcTextLine(const String& text, LInt maxWidth, LInt fontSize) const
{
    if (m_privateFont == NULL)
    {
        KLOG("FontAndroid::calcTextLine0");
        return 0;
    }
    //JNIEnv* env = m_privateFont->m_env;

    JNIEnv* env = yanbo::JNIUtil::getEnv();
    KLOG("FontAndroid::calcTextLine1");
    AutoJObject javaObject = m_privateFont->object(env);
    KLOG("FontAndroid::calcTextLine2");
    if (!javaObject.get())
        return 0;

    KLOG("FontAndroid::calcTextLine3");

    jstring strText = strToJstring(env, (const char*)text.GetBuffer());
    LInt longestWidth = env->CallIntMethod(
            javaObject.get(),
            m_privateFont->m_calcTextLine,
            strText,
            maxWidth,
            fontSize
    );

    env->DeleteLocalRef(strText);

    return longestWidth;
}

class AndroidFont : public LFont
{
public:
    AndroidFont(const LFont& font);
    virtual ~AndroidFont();

    virtual LInt getFontHeight() const;
    virtual LInt getFontWidth(LUint8 ch) const;
    virtual LInt getTextWidth(const String& text) const;
    virtual LInt calcTextLine(const String& text, LInt maxWidth) const;
    virtual LInt getLineSize() const;
    virtual LInt getLineWidth(LInt index) const;
    virtual LVoid getLineText(LInt index, String& text);

private:
    FontAndroid* m_fontPort;
};

AndroidFont::AndroidFont(const LFont& font)
    : LFont(font)
{
    m_fontPort = new FontAndroid();
}

AndroidFont::~AndroidFont()
{
	delete m_fontPort;
}

LInt AndroidFont::getFontHeight() const
{
    return m_fontPort->getFontHeight(m_size > 0 ? m_size : 12);
}

LInt AndroidFont::getFontWidth(LUint8 ch) const
{
    return m_fontPort->getFontWidth(ch, m_size > 0 ? m_size : 12);
}

LInt AndroidFont::getTextWidth(const String& text) const
{
    return m_fontPort->getTextWidth(text, m_size > 0 ? m_size : 12);
}

LInt AndroidFont::getLineSize() const
{
    return m_fontPort->getLineSize();
}

LInt AndroidFont::getLineWidth(LInt index) const
{
    return m_fontPort->getLineWidth(index);
}

LVoid AndroidFont::getLineText(LInt index, String& text)
{
	m_fontPort->getLineText(index, text);
}

LInt AndroidFont::calcTextLine(const String& text, LInt maxWidth) const
{
    return m_fontPort->calcTextLine(text, maxWidth, m_size);
}

LFont* LFont::create(const LFont& font)
{
    return new AndroidFont(font);
}

}
