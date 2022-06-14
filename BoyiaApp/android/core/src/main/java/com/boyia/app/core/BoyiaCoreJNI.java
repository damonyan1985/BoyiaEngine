package com.boyia.app.core;

import android.app.Activity;
import android.view.Surface;

import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.loader.job.JobScheduler;
import com.boyia.app.loader.mue.MainScheduler;
import com.boyia.app.loader.mue.MueTask;
import com.boyia.app.loader.mue.Subscriber;

import java.util.concurrent.atomic.AtomicBoolean;

public class BoyiaCoreJNI {
    private static AtomicBoolean sHasInit = new AtomicBoolean(false);

   static {
       System.loadLibrary("sdk_main");
   }

    public interface LibraryInitCallback {
        void initOk();
    }

    public static void initLibrary(LibraryInitCallback callback) {
        MueTask.create((Subscriber<String> subscriber) -> {
            /**
             * 判断boyia内核是否已经初始化过
             */
            if (sHasInit.compareAndSet(false, true)) {
                BoyiaUtils.loadLib();
            }
            // 这里处理了onComplete。subscribe才会调用襄阳的onComplete
            subscriber.onComplete();
        })
        .subscribeOn(JobScheduler.jobScheduler())
        .observeOn(MainScheduler.mainScheduler())
        .subscribe(() -> callback.initOk());
    }

    // Native Method Define
    public static native void nativeInitUIView(int width, int height, boolean isDebug);

    public static native void nativeOnDataSize(long size, long callback);

    public static native void nativeOnDataReceive(byte[] data, int length, long callback);

    public static native void nativeOnDataFinished(long callback);

    public static native void nativeHandleKeyEvent(int keyCode, int isDown);

    public static native void nativeDistroyUIView();

    public static native void nativeImageLoaded(long item);

    public static native void nativeHandleTouchEvent(int type, int x, int y);

    public static native void nativeOnLoadError(String error, long callback);

    public static native void nativeInitJNIContext(Activity context);

    public static native void nativeSetInputText(String text, long item);

    public static native void nativeVideoTextureUpdate(long item);

    public static native void nativeSetGLSurface(Surface surface);

    public static native void nativeResetGLSurface(Surface surface);

    public static native void nativeOnKeyboardShow(long item, int keyboardHeight);

    public static native void nativeOnKeyboardHide(long item, int keyboardHeight);

    public static native void nativeBoyiaSync();

    public static native void nativeCacheCode();
    
    public static native void nativePlatformViewUpdate(String viewId);

    // 此处初始化rust sdk
    public static native String nativeInitSdk();

    // 滚动接口
    public static native void nativeOnFling(int type1, int x1, int y1,
        int type2, int x2, int y2, float velocityX, float velocityY);

    // 启动boyia app
    public static native void nativeLaunchApp(int aid, String name, int version,
                                              String url, String cover);

    // boyia app api回调
    public static native void nativeApiCallback(String result, long callback);
}
