package com.boyia.app.core;

import android.view.Choreographer;

import com.boyia.app.loader.mue.MainScheduler;

/**
 * BoyiaSync use Android vsync to paint ui
 */
public class BoyiaSync {
    public static void awaitSync() {
        MainScheduler.mainScheduler().sendJob(() -> {
            Choreographer.getInstance().postFrameCallback((frameTimeNanos) ->
                    BoyiaCoreJNI.nativeBoyiaSync()
            );
        });
    }
}
