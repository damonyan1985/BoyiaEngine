package com.boyia.app.core;

import android.app.Activity;
import android.view.Surface;

import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.loader.job.JobScheduler;
import com.boyia.app.loader.jober.MainScheduler;
import com.boyia.app.loader.jober.Observable;
import com.boyia.app.loader.jober.Subscriber;

import java.util.concurrent.atomic.AtomicBoolean;

public class BoyiaCoreJNI {
    private static AtomicBoolean sHasInit = new AtomicBoolean(false);
    public interface LibraryInitCallback {
        void initOk();
    }

    public static void initLibrary(LibraryInitCallback callback) {
        Observable.create((Subscriber<String> subscriber) -> {
            /**
             * 判断boyia内核是否已经初始化过
             */
            if (sHasInit.compareAndSet(false, true)) {
                BoyiaUtils.loadLib();
            }
            subscriber.onComplete();
        })
        .subscribeOn(JobScheduler.getInstance())
        .observeOn(MainScheduler.mainScheduler())
        .subscribe(() -> callback.initOk());
    }

    // Native Method Define
    public static native void nativeInitUIView(int width, int height, boolean isDebug);

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

    public static native void nativeBoyiaSync(long item);

    public static native void nativeCacheCode();
}
