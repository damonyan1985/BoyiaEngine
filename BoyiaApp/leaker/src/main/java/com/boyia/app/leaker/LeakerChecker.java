package com.boyia.app.leaker;

import com.boyia.app.task.JobBase;
import com.boyia.app.task.JobScheduler;

import java.lang.ref.ReferenceQueue;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArraySet;

public class LeakerChecker {
    private ReferenceQueue mQueue;
    private final Set<String> mRetainedKeys;

    public LeakerChecker() {
        mQueue = new ReferenceQueue();
        mRetainedKeys = new CopyOnWriteArraySet<>();
    }

    public void watch(Object watchedRef){
        final String key = UUID.randomUUID().toString();
        final BoyiaWeakReference ref = new BoyiaWeakReference(watchedRef, key, mQueue);
        mRetainedKeys.add(key);
        JobScheduler.getInstance().sendJob(new JobBase(){
            @Override
            public void exec() {
                clearGC();
                removeRef();
                if (checkRef(key)) {
                    // 内存泄漏
                }
            }
        });
    }

    public boolean checkRef(final String key) {
        return mRetainedKeys.contains(key);
    }

    private void removeRef() {
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
