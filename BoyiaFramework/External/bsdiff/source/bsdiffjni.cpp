#ifndef BsdiffJNI_h
#define BsdiffJNI_h

#include <jni.h>

extern "C" 
{
int bspatch(const char *old_apk_path, const char *new_apk_path,
		const char *patch_file_path);
}

extern void nativeUpdatePatch(JNIEnv *env, jobject object, jstring oldApkPath,
		jstring newApkPath, jstring patchFilePath)
{
	const char *old_apk_path = env->GetStringUTFChars(oldApkPath, 0);
	const char *new_apk_path = env->GetStringUTFChars(newApkPath, 0);
	const char *patch_file_path = env->GetStringUTFChars(patchFilePath, 0);
	// do bspatch
	bspatch(old_apk_path, new_apk_path, patch_file_path);
	env->ReleaseStringUTFChars(oldApkPath, old_apk_path);
	env->ReleaseStringUTFChars(newApkPath, new_apk_path);
	env->ReleaseStringUTFChars(patchFilePath, patch_file_path);
}

#endif
