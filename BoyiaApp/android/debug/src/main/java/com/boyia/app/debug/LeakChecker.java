package com.boyia.app.debug;

import com.boyia.app.loader.job.JobScheduler;

import java.lang.ref.ReferenceQueue;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArraySet;

public class LeakChecker {
    private ReferenceQueue mQueue;
    private final Set<String> mRetainedKeys;

    public LeakChecker() {
        mQueue = new ReferenceQueue();
        mRetainedKeys = new CopyOnWriteArraySet<>();
    }

    public void watch(Object watchedRef) {
        final String key = UUID.randomUUID().toString();
        final BoyiaWeakReference ref = new BoyiaWeakReference(watchedRef, key, mQueue);
        mRetainedKeys.add(key);
        JobScheduler.jobScheduler().sendJob(() -> {
            try {
                // 延迟5秒
                Thread.sleep(5000);
                removeWeakRef();
                // 如果已经被释放，则无内存泄漏
                if (checkRef(ref)) {
                    return;
                }
                // 先强制GC清除一次
                clearGC();
                removeWeakRef();
                if (!checkRef(ref)) {
                    // 内存泄漏
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
}
