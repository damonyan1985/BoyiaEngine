#include "LGdi.h"
#include "AutoObject.h"
#include "JNIUtil.h"
#include "UIThread.h"
#include "CameraView.h"
#include "UIThreadClientMap.h"
#include <jni.h>

namespace util {

struct JCamera {
    jweak m_obj;
    jmethodID m_start;
    jmethodID m_startRecording;
    jmethodID m_stopRecording;
    jmethodID m_getCameraId;

    AutoJObject object(JNIEnv* env)
    {
        return getRealObject(env, m_obj);
    }
};

// Android平台摄像头实现
class CameraAndroid : public LCamera, public yanbo::UIThreadClient {
public:
    CameraAndroid(LVoid* view) 
        : m_view(view) 
    {
        JNIEnv* env = JNIUtil::getEnv();
        jclass clazz = JNIUtil::getJavaClassID("com/boyia/app/core/device/camera/BoyiaCamera");
        jmethodID constructMethod = env->GetMethodID(clazz, "<init>",
            "()V");
        jobject obj = env->NewObject(clazz, constructMethod);

        m_jcamera = new JCamera;
        m_jcamera->m_obj = env->NewGlobalRef(obj);
        m_jcamera->m_start = GetJMethod(env, clazz, "openCamera", "()V");
        m_jcamera->m_startRecording = GetJMethod(env, clazz, "startRecording", "()V");
        m_jcamera->m_stopRecording = GetJMethod(env, clazz, "stopRecording", "()V");
        m_jcamera->m_getCameraId = GetJMethod(env, clazz, "getCamerId", "()J");
    }

    // 启动摄像头
    virtual LVoid start() 
    {
        JNIEnv* env = JNIUtil::getEnv();
        AutoJObject javaObject = m_jcamera->object(env);
        if (!javaObject.get())
            return;

        env->CallVoidMethod(javaObject.get(), m_jcamera->m_start);
    }
    
    // 开始录制视频
    virtual LVoid startRecording()
    {
        JNIEnv* env = JNIUtil::getEnv();
        AutoJObject javaObject = m_jcamera->object(env);
        if (!javaObject.get())
            return;

        env->CallVoidMethod(javaObject.get(), m_jcamera->m_startRecording);
    }

    // 停止录制视频
    virtual LVoid stopRecording()
    {
        JNIEnv* env = JNIUtil::getEnv();
        AutoJObject javaObject = m_jcamera->object(env);
        if (!javaObject.get())
            return;

        env->CallVoidMethod(javaObject.get(), m_jcamera->m_stopRecording);
    }

    virtual LVoid onClientCallback()
    {
        yanbo::UIThread::instance()->drawUI(m_view);
    }

private:
    JCamera* m_jcamera;
    LVoid* m_view;
};

LCamera* LCamera::create(LVoid* view)
{
    return new CameraAndroid(view);
}

}