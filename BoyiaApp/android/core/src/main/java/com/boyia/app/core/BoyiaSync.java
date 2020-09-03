package com.boyia.app.core;

import android.view.Choreographer;

import com.boyia.app.loader.jober.MainScheduler;

/**
 * BoyiaSync use Android vsync to paint ui
 */
public class BoyiaSync {
    /**
     * ptr是native层传递过来的weakptr指针
     * 如WeakPtr<T>* ptr = new WeakPtr<T>(BoyiaPtr<T>());
     * Choreographer need looper
     * @param ptr
     */
    public static void awaitSync(long ptr) {
        MainScheduler.mainScheduler().sendJob(() -> {
            Choreographer.getInstance().postFrameCallback((frameTimeNanos) ->
                    BoyiaCoreJNI.nativeBoyiaSync(ptr)
            );
        });
    }
}
