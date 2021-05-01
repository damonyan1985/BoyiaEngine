#ifndef BoyiaViewCore_h
#define BoyiaViewCore_h

#include "AppManager.h"
#include "ArmFunction.h"
#include "AutoObject.h"
#include "BoyiaCore.h"
#include "BoyiaExecution.h"
#include "BoyiaSocket.h"
#include "FileUtil.h"
#include "GLContext.h"
#include "GraphicsContextGL.h"
#include "JNIUtil.h"
#include "LEvent.h"
#include "LGdi.h"
#include "LoaderAndroid.h"
#include "PixelRatio.h"
#include "PlatformBridge.h"
#include "SalLog.h"
#include "ShaderUtil.h"
#include "StringUtils.h"
#include "ThreadPool.h"
#include "UIThread.h"
#include "UIView.h"
#include <jni.h>

const char* kBoyiaUIViewClass = "com/boyia/app/core/BoyiaCoreJNI";
const char* kBoyiaUtilClass = "com/boyia/app/common/utils/BoyiaUtils";

enum KeyEventType {
    ACTION_ARROW_UP = 19,
    ACTION_ARROW_DOWN = 20,
    ACTION_ENTER = 66,
    ACTION_CENTER = 23,
};

bool JNI_LOG_ON = true;

static LVoid boyiaSocketTest()
{
    boyia::BoyiaSocket* socket = new boyia::BoyiaSocket(_CS("ws://192.168.0.10:6666"), kBoyiaNull);
}

static void nativeSetGLSurface(
    JNIEnv* env,
    jobject obj,
    jobject surface)
{
    util::LGraphicsContext* gc = yanbo::AppManager::instance()->uiThread()->graphics();
    static_cast<util::GraphicsContextGL*>(gc)->setContextWin(surface);
    yanbo::AppManager::instance()->uiThread()->initContext();
}

static void nativeResetGLSurface(
    JNIEnv* env,
    jobject obj,
    jobject surface)
{
    util::LGraphicsContext* gc = yanbo::AppManager::instance()->uiThread()->graphics();
    static_cast<util::GraphicsContextGL*>(gc)->setContextWin(surface);
    yanbo::AppManager::instance()->uiThread()->resetContext();
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
    LInt logicHeight = (1.0f * 720 / w) * h;
    yanbo::PixelRatio::setWindowSize(w, h);
    yanbo::PixelRatio::setLogicWindowSize(720, logicHeight);

    BOYIA_LOG("nativeInitUIView height=%d and logicHeight=", h, logicHeight);
    //LInt logicHeight = h;

    //mjs::BoyiaExecution exec;
    //exec.callCode();
    //android::CallStack callstack;
    //callstack.update();
    //callstack.log("BoyiaUI_Stack");

    JNI_LOG_ON = isDebug;
    //ArmMemeset(NULL, 0, 0);
    boyiaSocketTest();

    if (!FileUtil::isExist(yanbo::PlatformBridge::getAppRoot())) {
        FileUtil::createDirs(yanbo::PlatformBridge::getAppRoot());
    }
    FileUtil::printAllFiles(yanbo::PlatformBridge::getAppRoot());
    KLOG("TaskThread::run nativeInitUIView");
    //w = 720;
    //h = 1280;
    yanbo::ShaderUtil::setScreenSize(720, logicHeight);
    yanbo::AppManager::instance()->setViewport(LRect(0, 0, 720, logicHeight));
    yanbo::AppManager::instance()->start();
}

static void nativeDistroyUIView(JNIEnv* env, jobject obj)
{
    yanbo::AppManager::instance()->uiThread()->destroy();
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
    reinterpret_cast<yanbo::NetworkClient*>(callback)->onLoadError(yanbo::NetworkClient::kNetworkFileError);
}

static void nativeHandleTouchEvent(JNIEnv* env, jobject obj, jint type, jint x, jint y)
{
    yanbo::AppManager::instance()->handleTouchEvent(type, x, y);
}

static void nativeHandleKeyEvent(JNIEnv* env, jobject obj, jint keyCode, jint isDown)
{

    LKeyEvent::KeyEventType code = LKeyEvent::KEY_ARROW_DOWN;
    switch (keyCode) {
    case ACTION_ARROW_UP: {
        code = LKeyEvent::KEY_ARROW_UP;
    } break;
    case ACTION_ARROW_DOWN: {
        code = LKeyEvent::KEY_ARROW_DOWN;
    } break;
    }

    LKeyEvent* evt = new LKeyEvent(code, isDown);
    yanbo::UIThread::instance()->handleKeyEvent(evt);
}

static void nativeImageLoaded(JNIEnv* env, jobject obj, jlong item)
{
    yanbo::UIThread::instance()->clientCallback(item);
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
    //yanbo::UIThread::instance()->videoUpdate(item);
    yanbo::UIThread::instance()->clientCallback(item);
}

static void nativeOnKeyboardShow(JNIEnv* env, jobject obj, jlong item, jint keyboardHight)
{
    yanbo::UIThread::instance()->onKeyboardShow(item, yanbo::PixelRatio::viewY(keyboardHight));
}

static void nativeOnKeyboardHide(JNIEnv* env, jobject obj, jlong item, jint keyboardHight)
{
    yanbo::UIThread::instance()->onKeyboardHide(item, yanbo::PixelRatio::viewY(keyboardHight));
}

static void nativeBoyiaSync(JNIEnv* env, jobject obj)
{
    yanbo::AppManager::instance()->uiThread()->vsyncDraw();
}

static void nativeCacheCode(JNIEnv* env, jobject obj)
{
    if (FileUtil::isExist(yanbo::PlatformBridge::getInstructionEntryPath())) {
        return;
    }

    CacheVMCode(yanbo::AppManager::instance()->currentApp()->runtime()->vm());
}

static void nativePlatformViewUpdate(JNIEnv* env, jobject obj, jstring id)
{
    String viewId;
    util::jstringTostr(env, id, viewId);
    BOYIA_LOG("nativePlatformViewUpdate viewId=%s", GET_STR(viewId));
    yanbo::UIThread::instance()->platformViewUpdate(viewId);
    viewId.ReleaseBuffer();
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
    { "nativeBoyiaSync", "()V", (void*)nativeBoyiaSync },
    { "nativeCacheCode", "()V", (void*)nativeCacheCode },
    { "nativePlatformViewUpdate", "(Ljava/lang/String;)V", (void*)nativePlatformViewUpdate },
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

    return JNI_TRUE;
}

#endif
