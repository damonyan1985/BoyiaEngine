#ifndef MiniJniOnLoad_h
#define MiniJniOnLoad_h

#include "jni.h"
#include "AutoObject.h"
#include "JNIUtil.h"

int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods)
{
	jclass clazz = env->FindClass(className);
	if (clazz == 0)
	{
		return JNI_FALSE;
	}

	if (env->RegisterNatives(clazz, gMethods, numMethods) < 0)
	{
		env->DeleteLocalRef(clazz);
		return JNI_FALSE;
	}

	env->DeleteLocalRef(clazz);
	return JNI_TRUE;
}

extern int registerBrowserNatives(JNIEnv* env);

struct RegistrationMethod {
    const char* name;
    int (*func)(JNIEnv*);
};

static RegistrationMethod gMiniRegMethods[] = {
	{"registerBrowserNatives", registerBrowserNatives},
};

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	yanbo::JNIUtil::setJavaVM(vm);

    JNIEnv* env = 0;
    jint result = -1;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) 
    {
        return result;
    }
    
    const RegistrationMethod* method = gMiniRegMethods;
    const RegistrationMethod* end = sizeof(gMiniRegMethods) == 0 ? method : method + sizeof(gMiniRegMethods)/sizeof(RegistrationMethod);
    
    while (method != end) 
    {
        if (method->func(env) < 0) 
        {
            return result;
        }
        method++;
    }
    
    return JNI_VERSION_1_4;
}

#endif
