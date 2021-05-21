/*
 * GraphicsContextAndroid.cpp
 *
 *  Created on: 2012-5-10
 *      Author: yanbo
 *  2d graphic api instance by java
 */

#include "GraphicsContextAndroid.h"
#include "AutoObject.h"
#include "FontAndroid.h"
#include "ImageAndroid.h"
#include "JNIUtil.h"
#include "LColorUtil.h"
#include "SalLog.h"
#include "StringUtils.h"

#include <android/log.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

namespace util {

struct JGraphicsContext {
    //JNIEnv*   m_env;
    jweak m_obj;
    jmethodID m_drawLineI4;
    jmethodID m_drawLineP2;
    jmethodID m_drawRectI4;
    jmethodID m_drawRectR;
    jmethodID m_drawTextSP;
    jmethodID m_drawTextSR;
    jmethodID m_setBrushColorI;
    jmethodID m_setPenColorI;
    jmethodID m_setBrushStyleI;
    jmethodID m_setPenStyleI;
    jmethodID m_setFontI;
    jmethodID m_drawBitmap;
    jmethodID m_drawJavaBitmap;
    AutoJObject object(JNIEnv* env)
    {
        return getRealObject(env, m_obj);
    }
};

void GraphicsContextAndroid::initGraphics()
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    jclass clazz = yanbo::JNIUtil::getJavaClassID("com/boyia/app/core/BoyiaGraphics");
    jmethodID constructMethod = env->GetMethodID(clazz, "<init>",
        "()V");
    jobject obj = env->NewObject(clazz, constructMethod);

    m_privateGc = new JGraphicsContext;
    m_privateGc->m_obj = env->NewGlobalRef(obj);
    m_privateGc->m_drawLineI4 = GetJMethod(env, clazz, "drawLine", "(IIII)V");
    m_privateGc->m_drawRectI4 = GetJMethod(env, clazz, "drawLine", "(IIII)V");
    m_privateGc->m_drawTextSP = GetJMethod(env, clazz, "drawText", "(Ljava/lang/String;II)V");
    m_privateGc->m_drawTextSR = GetJMethod(env, clazz, "drawText", "(Ljava/lang/String;IIIII)V");
    m_privateGc->m_drawRectI4 = GetJMethod(env, clazz, "drawRect", "(IIII)V");
    m_privateGc->m_setBrushColorI = GetJMethod(env, clazz, "setBrushColor", "(I)V");
    m_privateGc->m_setPenColorI = GetJMethod(env, clazz, "setPenColor", "(I)V");
    m_privateGc->m_setPenStyleI = GetJMethod(env, clazz, "setPenStyle", "(I)V");
    m_privateGc->m_setBrushStyleI = GetJMethod(env, clazz, "setBrushStyle", "(I)V");
    m_privateGc->m_setFontI = GetJMethod(env, clazz, "setFont", "(II)V");
    m_privateGc->m_drawBitmap = GetJMethod(env, clazz, "drawBitmap", "(Lcom/boyia/app/core/BoyiaBitmap;IIII)V");
    m_privateGc->m_drawJavaBitmap = GetJMethod(env, clazz, "drawBitmap", "(Landroid/graphics/Bitmap;IIIII)V");

    env->DeleteLocalRef(obj);
    env->DeleteLocalRef(clazz);
}

GraphicsContextAndroid::GraphicsContextAndroid()
    : m_privateGc(NULL)
{
}

GraphicsContextAndroid::~GraphicsContextAndroid()
{
    if (m_privateGc != NULL) {
        JNIEnv* env = yanbo::JNIUtil::getEnv();
        env->DeleteGlobalRef(m_privateGc->m_obj);
        delete m_privateGc;
    }
}

LVoid GraphicsContextAndroid::restore()
{
}

LVoid GraphicsContextAndroid::reset()
{
}

LVoid GraphicsContextAndroid::save()
{
}

LVoid GraphicsContextAndroid::drawLine(const LPoint& p1, const LPoint& p2)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawLineI4,
        p1.iX, p1.iY, p2.iX, p2.iY);
}

LVoid GraphicsContextAndroid::drawLine(LInt x0, LInt y0, LInt x1, LInt y1)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawLineI4,
        x0, y0, x1, y1);
}

LVoid GraphicsContextAndroid::drawRect(const LRect& aRect)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawRectI4,
        aRect.iTopLeft.iX, aRect.iTopLeft.iY, aRect.GetWidth(), aRect.GetHeight());
}

LVoid GraphicsContextAndroid::drawRect(LInt x, LInt y, LInt w, LInt h)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawRectI4,
        x, y, w, h);
}

LVoid GraphicsContextAndroid::drawEllipse(const LRect& aRect)
{
}

LVoid GraphicsContextAndroid::drawRoundRect(const LRect& aRect, const LSize& aCornerSize)
{
}

LVoid GraphicsContextAndroid::setHtmlView(LVoid* item)
{
}

LVoid GraphicsContextAndroid::drawText(const String& aText, const LRect& aRect)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;

    //String astr;
    //util::StringUtils::strWtoStr(aText, astr);
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawTextSP,
        strToJstring(env, aText), aRect.iTopLeft.iX, aRect.iTopLeft.iY);
}

LVoid GraphicsContextAndroid::drawText(const String& aText, const LPoint& aPoint)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;

    //String astr;
    //util::StringUtils::strWtoStr(aText, astr);
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawTextSR,
        strToJstring(env, aText), aPoint.iX, aPoint.iY);
}

LVoid GraphicsContextAndroid::drawImage(const LPoint& aTopLeft, const LImage* aBitmap)
{
}

LVoid GraphicsContextAndroid::drawImage(const LImage* image)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    //JNIEnv* env = getJNIEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;

    KLOG("GraphicsContextAndroid::drawBitmap");
    const ImageAndroid* bitmap = static_cast<const ImageAndroid*>(image);
    AutoJObject javaBitmap = bitmap->getJavaBitmap();
    KLOG("GraphicsContextAndroid::drawBitmap1");

    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawBitmap,
        javaBitmap.get(),
        bitmap->rect().iTopLeft.iX,
        bitmap->rect().iTopLeft.iY,
        bitmap->rect().GetWidth(),
        bitmap->rect().GetHeight());

    KLOG("GraphicsContextAndroid::drawBitmap2");
}

LVoid GraphicsContextAndroid::drawImage(const LRect& aDestRect, const LImage* aSource, const LRect& aSourceRect)
{
}

LVoid GraphicsContextAndroid::setBrushStyle(BrushStyle aBrushStyle)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_setBrushStyleI,
        (int)aBrushStyle);
}

LVoid GraphicsContextAndroid::setPenStyle(PenStyle aPenStyle)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_setPenStyleI,
        (int)aPenStyle);
}

LVoid GraphicsContextAndroid::setBrushColor(const LColor& aColor)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_setBrushColorI,
        LColorUtil::rgb(aColor.m_red, aColor.m_green, aColor.m_blue));
}

LVoid GraphicsContextAndroid::setPenColor(const LColor& aColor)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_setPenColorI,
        LColorUtil::rgb(aColor.m_red, aColor.m_green, aColor.m_blue));
}

LVoid GraphicsContextAndroid::setFont(const LFont& font)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_setFontI,
        font.getFontSize(), font.getFontStyle());
}

LVoid GraphicsContextAndroid::drawVideo(const LRect& rect, const LMediaPlayer* mp)
{
}

LVoid GraphicsContextAndroid::drawText(const String& text, const LRect& rect, TextAlign align)
{
    JNIEnv* env = yanbo::JNIUtil::getEnv();
    AutoJObject javaObject = m_privateGc->object(env);
    if (!javaObject.get())
        return;

    //String astr;
    //util::StringUtils::strWtoStr(text, astr);
    KFORMATLOG("GraphicsContextAndroid_drawText string=%s", (const char*)text.GetBuffer());
    env->CallVoidMethod(javaObject.get(), m_privateGc->m_drawTextSR,
        strToJstring(env, text),
        rect.iTopLeft.iX,
        rect.iTopLeft.iY,
        rect.iBottomRight.iX,
        rect.iBottomRight.iY,
        (jint)align);
}

LVoid GraphicsContextAndroid::clipRect(const LRect& rect)
{
}

LVoid GraphicsContextAndroid::submit()
{
}

#if ENABLE(BOYIA_PLATFORM_VIEW)
LVoid GraphicsContextAndroid::drawPlatform(const LRect& rect, LVoid* platformView)
{
}
#endif
}
