package com.boyia.app.debug;

import android.app.Activity;
import android.app.Application;
import android.os.Bundle;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.job.JobScheduler;

import java.lang.ref.ReferenceQueue;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArraySet;

public class LeakChecker {
    private static final String TAG = "LeakChecker";
    private static final int CLEAR_TIME = 5000;
    private ReferenceQueue mQueue;
    private final Set<String> mRetainedKeys;

    public LeakChecker() {
        mQueue = new ReferenceQueue();
        mRetainedKeys = new CopyOnWriteArraySet<>();
    }

    public void watch(Object watchedRef, LeakCallback callback) {
        final String key = UUID.randomUUID().toString();
        final BoyiaWeakReference ref = new BoyiaWeakReference(watchedRef, key, mQueue);
        mRetainedKeys.add(key);
        JobScheduler.jobScheduler().sendJob(() -> {
            try {
                // 延迟5秒
                Thread.sleep(CLEAR_TIME);
                removeWeakRef();
                // 如果已经被释放，则无内存泄漏
                if (checkRef(ref)) {
                    return;
                }
                // 先强制GC清除一次
                clearGC();
                removeWeakRef();
                if (!checkRef(ref) && callback != null) {
                    // 通知内存泄漏
                    callback.onLeak();
                }
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        });
    }

    public boolean checkRef(BoyiaWeakReference ref) {
        return !mRetainedKeys.contains(ref.mKey);
    }

    private void removeWeakRef() {
        BoyiaWeakReference ref;
        while ((ref = (BoyiaWeakReference) mQueue.poll()) != null) {
            mRetainedKeys.remove(ref.mKey);
        }
    }

    private void clearGC() {
        Runtime.getRuntime().gc();
        try {
            Thread.sleep(100);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        System.runFinalization();
    }

    public static void install(Application app) {
        app.registerActivityLifecycleCallbacks(new Application.ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(Activity activity, Bundle savedInstanceState) {
            }

            @Override
            public void onActivityStarted(Activity activity) {
            }

            @Override
            public void onActivityResumed(Activity activity) {
            }

            @Override
            public void onActivityPaused(Activity activity) {
            }

            @Override
            public void onActivityStopped(Activity activity) {
            }

            @Override
            public void onActivitySaveInstanceState(Activity activity, Bundle outState) {
            }

            /**
             * app进程中的activity被杀死也会回调
             * @param activity
             */
            @Override
            public void onActivityDestroyed(Activity activity) {
                BoyiaLog.d(TAG, String.format("activity destroyed name = %s and action = %s", activity.getClass().getCanonicalName(), activity.getIntent().getAction()));
                //BoyiaLog.d(TAG, String.format("activity destroyed action = %s", activity.getIntent().getAction()));
                // 监控activity是否被销毁
                new LeakChecker().watch(activity, () -> {
                    // 此处可以上报内存泄露
                    BoyiaLog.d(TAG, String.format("Leak activity %s", activity.getClass().getCanonicalName()));
                });
            }
        });
    }
}
