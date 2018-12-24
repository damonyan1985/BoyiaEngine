package com.boyia.app;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.ProcessUtil;
import com.squareup.leakcanary.LeakCanary;
import com.squareup.leakcanary.RefWatcher;
import com.boyia.app.common.BoyiaApplication;

public class BoyiaMultiApplication extends BoyiaApplication {
    private RefWatcher mWatcher;

    @Override
    public void onCreate() {
        super.onCreate();
        mWatcher = LeakCanary.install(this);
        // Multi Process share the same Application
        BoyiaLog.d("BoyiaApp", "BoyiaProcess="+ProcessUtil.getCurrentProcessName(this));
    }

    public RefWatcher watcher() {
        return mWatcher;
    }
}
