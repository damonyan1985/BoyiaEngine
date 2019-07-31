#ifndef BoyiaViewCore_h
#define BoyiaViewCore_h

#include "ArmFunction.h"
#include "AutoObject.h"
#include "BoyiaExecution.h"
#include "BoyiaLoader.h"
#include "BoyiaThread.h"
#include "FileUtil.h"
#include "GLContext.h"
#include "GraphicsContextGL.h"
#include "JNIUtil.h"
#include "LEvent.h"
#include "LoaderAndroid.h"
#include "MiniThread.h"
#include "MiniThreadPool.h"
#include "SalLog.h"
#include "ShaderUtil.h"
#include "StringUtils.h"
#include "UIThread.h"
#include "UIView.h"
#include <CallStack.h>
#include <jni.h>

const char* kBoyiaUIViewClass = "com/boyia/app/core/BoyiaUIView";
const char* kBoyiaUtilClass = "com/boyia/app/common/utils/BoyiaUtils";

enum KeyEventType {
    ACTION_ARROW_UP = 19,
    ACTION_ARROW_DOWN = 20,
    ACTION_ENTER = 66,
    ACTION_CENTER = 23,
};

bool JNI_LOG_ON = true;

extern void nativeUpdatePatch(
    JNIEnv* env, jobject object,
    jstring oldApkPath, jstring newApkPath, jstring patchFilePath);

static void nativeSetGLSurface(
    JNIEnv* env,
    jobject obj,
    jobject surface)
{
    yanbo::UIThread::instance()->initContext(surface);
}

static void nativeResetGLSurface(
    JNIEnv* env,
    jobject obj,
    jobject surface)
{
    yanbo::UIThread::instance()->resetContext(surface);
}

extern LVoid TestLoadUrl();
extern LVoid TestThread();
static void nativeInitUIView(
    JNIEnv* env,
    jobject obj,
    jint w,
    jint h,
    jboolean isDebug)
{
    //mjs::BoyiaExecution exec;
    //exec.callCode();
    //android::CallStack callstack;
    //callstack.update();
    //callstack.log("BoyiaUI_Stack");

    JNI_LOG_ON = isDebug;
    //ArmMemeset(NULL, 0, 0);
    FileUtil::printAllFiles("/data/data/com.boyia.app/files/");
    KLOG("MiniTaskThread::run nativeInitUIView");
    w = 720;
    h = 1280;
    yanbo::ShaderUtil::setScreenSize(w, h);
    util::GraphicsContextGL* gc = new util::GraphicsContextGL();
    yanbo::LoaderAndroid* loader = new yanbo::LoaderAndroid();
    //yanbo::BoyiaLoader* loader = new yanbo::BoyiaLoader;
    loader->initLoader();
    yanbo::UIView::getInstance()->setClientRange(LRect(0, 0, w, h));
    yanbo::UIView::getInstance()->setComponents(loader, gc, NULL);
    TestLoadUrl();
    TestThread();
}

static void nativeDistroyUIView(JNIEnv* env, jobject obj)
{
    yanbo::BoyiaThread::instance()->destroy();
}

static void nativeOnDataReceive(JNIEnv* env, jobject obj, jbyteArray byteArray, jint len, jlong callback)
{
    jbyte* bytes = env->GetByteArrayElements(byteArray, 0);
    LByte* buffer = new LByte[len];
    LMemcpy(buffer, bytes, len);
    env->ReleaseByteArrayElements(byteArray, bytes, 0);
    reinterpret_cast<yanbo::NetworkClient*>(callback)->onDataReceived(buffer, len);
}

static void nativeOnDataFinished(JNIEnv* env, jobject obj, jlong callback)
{
    reinterpret_cast<yanbo::NetworkClient*>(callback)->onLoadFinished();
}

static void nativeOnLoadError(JNIEnv* env, jobject obj, jstring error, jlong callback)
{
    reinterpret_cast<yanbo::NetworkClient*>(callback)->onLoadError(yanbo::NetworkClient::NETWORK_FILE_ERROR);
}

static void nativeHandleTouchEvent(JNIEnv* env, jobject obj, jint type, jint x, jint y)
{
    if (!yanbo::UIView::getInstance()->canHit()) {
        return;
    }
    LTouchEvent* evt = new LTouchEvent;

    evt->m_type = 1 << type;
    evt->m_position.Set(yanbo::ShaderUtil::viewX(x), yanbo::ShaderUtil::viewY(y));
    KLOG("nativeHandleTouchEvent");
    yanbo::UIThread::instance()->handleTouchEvent(evt);
}

static void nativeHandleKeyEvent(JNIEnv* env, jobject obj, jint keyCode, jint isDown)
{

    LKeyEvent::KeyEventType mKeyCode = LKeyEvent::KEY_ARROW_DOWN;
    switch (keyCode) {
    case ACTION_ARROW_UP: {
        mKeyCode = LKeyEvent::KEY_ARROW_UP;
    } break;
    case ACTION_ARROW_DOWN: {
        mKeyCode = LKeyEvent::KEY_ARROW_DOWN;
    } break;
    }

    LKeyEvent* evt = new LKeyEvent(mKeyCode, isDown);
    yanbo::UIThread::instance()->handleKeyEvent(evt);
}

static void nativeImageLoaded(JNIEnv* env, jobject obj, jlong item)
{
    yanbo::UIThread::instance()->imageLoaded(item);
}

static void nativeInitJNIContext(JNIEnv* env, jobject obj, jobject context)
{
    yanbo::JNIUtil::setClassLoaderFrom(context);
}

static void nativeSetInputText(JNIEnv* env, jobject obj, jstring text, jlong item)
{
    String result;
    util::jstringTostr(env, text, result);
    KFORMATLOG("nativeSetInputText text=%s", (const char*)result.GetBuffer());
    yanbo::UIThread::instance()->setInputText(result, item);
    result.ReleaseBuffer();
}

static void nativeVideoTextureUpdate(JNIEnv* env, jobject obj, jlong item)
{
    yanbo::UIThread::instance()->videoUpdate(item);
}

static void nativeOnKeyboardShow(JNIEnv* env, jobject obj, jlong item, jint keyboardHight)
{
    yanbo::UIThread::instance()->onKeyboardShow(item, yanbo::ShaderUtil::viewY(keyboardHight));
}

static void nativeOnKeyboardHide(JNIEnv* env, jobject obj, jlong item, jint keyboardHight)
{
    yanbo::UIThread::instance()->onKeyboardHide(item, yanbo::ShaderUtil::viewY(keyboardHight));
}

static JNINativeMethod sUIViewMethods[] = {
    { "nativeInitUIView", "(IIZ)V", (void*)nativeInitUIView },
    { "nativeOnDataReceive", "([BIJ)V", (void*)nativeOnDataReceive },
    { "nativeOnDataFinished", "(J)V", (void*)nativeOnDataFinished },
    { "nativeOnLoadError", "(Ljava/lang/String;J)V", (void*)nativeOnLoadError },
    { "nativeDistroyUIView", "()V", (void*)nativeDistroyUIView },
    { "nativeHandleKeyEvent", "(II)V", (void*)nativeHandleKeyEvent },
    { "nativeImageLoaded", "(J)V", (void*)nativeImageLoaded },
    { "nativeSetInputText", "(Ljava/lang/String;J)V", (void*)nativeSetInputText },
    { "nativeVideoTextureUpdate", "(J)V", (void*)nativeVideoTextureUpdate },
    { "nativeHandleTouchEvent", "(III)V", (void*)nativeHandleTouchEvent },
    { "nativeInitJNIContext", "(Landroid/app/Activity;)V", (void*)nativeInitJNIContext },
    { "nativeSetGLSurface", "(Landroid/view/Surface;)V", (void*)nativeSetGLSurface },
    { "nativeResetGLSurface", "(Landroid/view/Surface;)V", (void*)nativeResetGLSurface },
    { "nativeOnKeyboardShow", "(JI)V", (void*)nativeOnKeyboardShow },
    { "nativeOnKeyboardHide", "(JI)V", (void*)nativeOnKeyboardHide },
};

static JNINativeMethod sUtilMethods[] = {
    { "nativeUpdatePatch", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void*)nativeUpdatePatch },
};

extern int registerNativeMethods(JNIEnv* env, const char* className,
    JNINativeMethod* methods, int numMethods);

/*
* Register native methods for all classes we know about.
*/
int registerUIViewNatives(JNIEnv* env)
{
    if (!registerNativeMethods(env, kBoyiaUIViewClass, sUIViewMethods,
            sizeof(sUIViewMethods) / sizeof(sUIViewMethods[0]))) {
        return JNI_FALSE;
    }

    if (!registerNativeMethods(env, kBoyiaUtilClass, sUtilMethods,
            sizeof(sUtilMethods) / sizeof(sUtilMethods[0]))) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

#endif