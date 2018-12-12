package com.boyia.app;

import com.squareup.leakcanary.LeakCanary;
import com.squareup.leakcanary.RefWatcher;

public class BoyiaMultiApplication extends BoyiaApplication {
    private RefWatcher mWatcher;

    @Override
    public void onCreate() {
        super.onCreate();
        mWatcher = LeakCanary.install(this);
    }

    public RefWatcher watcher() {
        return mWatcher;
    }
}
