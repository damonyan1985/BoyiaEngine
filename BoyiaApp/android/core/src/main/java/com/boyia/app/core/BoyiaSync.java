package com.boyia.app.core;

import android.view.Choreographer;

/**
 * BoyiaSync use Android vsync to paint ui
 */
public class BoyiaSync {
    /**
     * ptr是native层传递过来的weakptr指针
     * 如WeakPtr<T>* ptr = new WeakPtr<T>(BoyiaPtr<T>());
     * @param ptr
     */
    public static void awaitSync(long ptr) {
        Choreographer.getInstance().postFrameCallback((callback) -> {
            BoyiaCoreJNI.nativeBoyiaSync(ptr);
        });
    }
}
