#ifndef BoyiaViewCore_h
#define BoyiaViewCore_h

#include "UIView.h"
#include "GraphicsContextGL.h"
#include "AutoObject.h"
#include "StringUtils.h"
#include "LoaderAndroid.h"
#include "SalLog.h"
#include "LEvent.h"
#include "MiniThread.h"
#include "MiniThreadPool.h"
#include "JNIUtil.h"
#include "UIViewThread.h"
#include "ArmFunction.h"
#include "BoyiaExecution.h"
#include "GLContext.h"
#include "PaintThread.h"
#include "ShaderUtil.h"
#include "FileUtil.h"
#include <jni.h>
#include <CallStack.h>

const char* kBoyiaUIViewClass = "com/boyia/app/core/BoyiaUIView";

enum KeyEventType
{
	ACTION_ARROW_UP = 19,
    ACTION_ARROW_DOWN = 20,
    ACTION_ENTER = 66,
    ACTION_CENTER = 23,
};

bool JNI_LOG_ON = true;

static void nativeSetGLSurface(
		JNIEnv* env,
		jobject obj,
		jobject surface)
{
    yanbo::PaintThread::instance()->initContext(surface);
}

static void nativeResetGLSurface(
		JNIEnv* env,
		jobject obj,
		jobject surface)
{
    yanbo::PaintThread::instance()->resetContext(surface);
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
    loader->initLoader();
    yanbo::UIView::getInstance()->setClientRange(LRect(0, 0, w, h));
    yanbo::UIView::getInstance()->setComponents(loader, gc, NULL);
    TestLoadUrl();
    TestThread();
}

static void nativeDistroyUIView(JNIEnv* env, jobject obj)
{
	yanbo::UIViewThread::instance()->destroy();
}

static void nativeLoadUrl(JNIEnv* env, jobject obj, jstring s)
{
#if 1
	String url;
	util::jstringTostr(env, s, url);
	//yanbo::UIView::getInstance()->loadPage(url);
    yanbo::UIViewThread::instance()->load(url);
	url.ReleaseBuffer();
#endif

#if 0
    String htmlSource = _CS("<html>"
    		                 "<head><style>"
    		                 "body{background-color:#ffffff;font-size:60px;color:#0000ff;}"
    		                 ".first{border: 1 solid #ff0000;background-color:#00ff00;color:#e32ede;}"
    		                 "</style></head>"
    		                 "<body>"
    		                 "<p>测试 my browser</p>"
    		                 "<p class=\"first\">测试CSS的CLASS</p>"
    		                 "</body></html>");
    yanbo::UIView::getInstance()->loadString(htmlSource);
#endif
}

static void nativeOnDataReceive(JNIEnv* env, jobject obj, jstring s)
{
	String result;
	util::jstringTostr(env, s, result);
	yanbo::UIView::getInstance()->getLoader()->onDataReceived(result);
	result.ReleaseBuffer();
}

static void nativeOnDataFinished(JNIEnv* env, jobject obj, jstring s, jlong callback)
{
	String result;
	util::jstringTostr(env, s, result);
	yanbo::UIViewThread::instance()->loadFinished(result, callback);
	result.ReleaseBuffer();
}

static void nativeOnLoadError(JNIEnv* env, jobject obj, jstring error, jlong callback)
{
	String result;
	util::jstringTostr(env, error, result);
	yanbo::UIViewThread::instance()->loadError(callback, 0);
}

static void nativeUIViewDraw(JNIEnv* env, jobject obj)
{
	yanbo::UIViewThread::instance()->draw(0);
}

static void nativeHandleTouchEvent(JNIEnv* env, jobject obj, jint type, jint x, jint y)
{
	LTouchEvent* evt = new LTouchEvent;

	evt->m_type = 1 << type;
    evt->m_position.Set(yanbo::ShaderUtil::viewX(x), yanbo::ShaderUtil::viewY(y));
    KLOG("nativeHandleTouchEvent");
    yanbo::PaintThread::instance()->handleTouchEvent(evt);
}

static void nativeHandleKeyEvent(JNIEnv* env, jobject obj, jint keyCode, jint isDown)
{

    LKeyEvent::KeyEventType mKeyCode = LKeyEvent::KEY_ARROW_DOWN;
    switch (keyCode)
    {
    case ACTION_ARROW_UP:
    	 {
    	    mKeyCode = LKeyEvent::KEY_ARROW_UP;
    	 }
    	 break;
    case ACTION_ARROW_DOWN:
    	 {
    	    mKeyCode = LKeyEvent::KEY_ARROW_DOWN;
    	 }
    	 break;
    }

    LKeyEvent* evt = new LKeyEvent(mKeyCode, isDown);
    yanbo::UIViewThread::instance()->handleKeyEvent(evt);
}

static void nativeImageLoaded(JNIEnv* env, jobject obj, jlong item)
{
	yanbo::UIViewThread::instance()->imageLoaded(item);
}

static void nativeInitJNIContext(JNIEnv*  env, jobject obj, jobject context)
{
    yanbo::JNIUtil::setClassLoaderFrom(context);
}

static void nativeSetInputText(JNIEnv*  env, jobject obj, jstring text, jlong item)
{
	String result;
	util::jstringTostr(env, text, result);
	KFORMATLOG("nativeSetInputText text=%s", (const char*)result.GetBuffer());
	yanbo::PaintThread::instance()->setInputText(result, item);
	result.ReleaseBuffer();
}

static void nativeVideoTextureUpdate(JNIEnv*  env, jobject obj, jlong item)
{
	yanbo::PaintThread::instance()->videoUpdate(item);
}

static JNINativeMethod sUIViewMethods[] = {
	{"nativeInitUIView", "(IIZ)V", (void*)nativeInitUIView},
	{"nativeLoadUrl", "(Ljava/lang/String;)V", (void*)nativeLoadUrl},
	{"nativeOnDataReceive", "(Ljava/lang/String;)V", (void*)nativeOnDataReceive},
	{"nativeOnDataFinished", "(Ljava/lang/String;J)V", (void*)nativeOnDataFinished},
	{"nativeOnLoadError", "(Ljava/lang/String;J)V", (void*)nativeOnLoadError},
	{"nativeUIViewDraw", "()V", (void*)nativeUIViewDraw},
	{"nativeDistroyUIView", "()V", (void*)nativeDistroyUIView},
	{"nativeHandleKeyEvent", "(II)V", (void*)nativeHandleKeyEvent},
	{"nativeImageLoaded", "(J)V", (void*)nativeImageLoaded},
	{"nativeSetInputText", "(Ljava/lang/String;J)V", (void*)nativeSetInputText},
	{"nativeVideoTextureUpdate", "(J)V", (void*)nativeVideoTextureUpdate},
	{"nativeHandleTouchEvent", "(III)V", (void*)nativeHandleTouchEvent},
	{"nativeInitJNIContext", "(Lcom/boyia/app/base/BaseActivity;)V", (void*)nativeInitJNIContext},
	{"nativeSetGLSurface", "(Landroid/view/Surface;)V", (void*)nativeSetGLSurface},
	{"nativeResetGLSurface", "(Landroid/view/Surface;)V", (void*)nativeResetGLSurface},
};

extern int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* methods, int numMethods);

/*
* Register native methods for all classes we know about.
*/
int registerUIViewNatives(JNIEnv* env)
{
	if (!registerNativeMethods(env, kBoyiaUIViewClass, sUIViewMethods,
                                 sizeof(sUIViewMethods) / sizeof(sUIViewMethods[0])))
	{
		return JNI_FALSE;
	}

	return JNI_TRUE;
}

#endif
