package com.boyia.app;

import android.app.Activity;
import android.os.Bundle;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.ProcessUtil;
import com.boyia.app.update.DownloadUtil;
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
        DownloadUtil.getDownloadTableName();
        BoyiaLog.d("BoyiaApp", "BoyiaProcess="+ProcessUtil.getCurrentProcessName(this));
        registerActivityLifecycleCallbacks(new ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(Activity activity, Bundle bundle) {
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
            public void onActivitySaveInstanceState(Activity activity, Bundle bundle) {
            }

            @Override
            public void onActivityDestroyed(Activity activity) {
            }
        });
    }

    public RefWatcher watcher() {
        return mWatcher;
    }
}
