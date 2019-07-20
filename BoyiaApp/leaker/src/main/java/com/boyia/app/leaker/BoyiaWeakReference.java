package com.boyia.app.leaker;

import java.lang.ref.ReferenceQueue;
import java.lang.ref.WeakReference;

public class BoyiaWeakReference extends WeakReference<Object> {
    public final String mKey;

    public BoyiaWeakReference(Object referent, String key,
                              ReferenceQueue<Object> referenceQueue) {
        super(referent, referenceQueue);
        mKey = key;
    }
}
