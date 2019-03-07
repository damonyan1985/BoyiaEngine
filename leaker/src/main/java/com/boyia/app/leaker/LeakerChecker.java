package com.boyia.app.leaker;

import java.lang.ref.Reference;
import java.lang.ref.ReferenceQueue;

public class LeakerChecker {
    private ReferenceQueue mQueue;

    public LeakerChecker() {
        mQueue = new ReferenceQueue();
    }
}
