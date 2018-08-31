#ifndef BsdiffJNI_h
#define BsdiffJNI_h

#include <jni.h>

extern int bspatch(const char *old_apk_path, const char *new_apk_path,
		const char *patch_file_path);

void Java_com_mini_app_utils_MiniUtils_nativeUpdatePatch(JNIEnv *env, jobject object, jstring oldApkPath,
		jstring newApkPath, jstring patchFilePath)
{
	const char *old_apk_path = (*env)->GetStringUTFChars(env, oldApkPath, 0);
	const char *new_apk_path = (*env)->GetStringUTFChars(env, newApkPath, 0);
	const char *patch_file_path = (*env)->GetStringUTFChars(env, patchFilePath, 0);
	// do bspatch
	bspatch(old_apk_path, new_apk_path, patch_file_path);
	(*env)->ReleaseStringUTFChars(env, oldApkPath, old_apk_path);
	(*env)->ReleaseStringUTFChars(env, newApkPath, new_apk_path);
	(*env)->ReleaseStringUTFChars(env, patchFilePath, patch_file_path);
}

#endif
