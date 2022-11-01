package com.boyia.app.shell;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.multidex.MultiDex;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.debug.LeakChecker;
import com.boyia.app.shell.home.HomeModule;
import com.boyia.app.shell.ipc.BoyiaIPCModule;
import com.boyia.app.shell.login.LoginModule;
import com.boyia.app.shell.module.ModuleManager;
import com.boyia.app.shell.search.SearchModule;
import com.boyia.app.shell.setting.BoyiaSettingModule;
import com.umeng.commonsdk.UMConfigure;

public class BoyiaApplication extends BaseApplication {
    private static final String TAG = "BoyiaApplication";
    // base为ContextImpl
    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        MultiDex.install(base);
        // 监控APP内activity销毁
        registerActivityLifecycleCallbacks(new ActivityLifecycleCallbacks() {
            @Override
            public void onActivityCreated(@NonNull Activity activity, @Nullable Bundle savedInstanceState) {
            }

            @Override
            public void onActivityStarted(@NonNull Activity activity) {
            }

            @Override
            public void onActivityResumed(@NonNull Activity activity) {
            }

            @Override
            public void onActivityPaused(@NonNull Activity activity) {
            }

            @Override
            public void onActivityStopped(@NonNull Activity activity) {
            }

            @Override
            public void onActivitySaveInstanceState(@NonNull Activity activity, @NonNull Bundle outState) {
            }

            /**
             * app进程中的activity被杀死也会回调
             * @param activity
             */
            @Override
            public void onActivityDestroyed(@NonNull Activity activity) {
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

    @Override
    public void onCreate() {
        super.onCreate();
        initModule();
        //TTAdManagerHolder.init(this);
        UMConfigure.setLogEnabled(true);
        UMConfigure.init(this, null, null,
                UMConfigure.DEVICE_TYPE_PHONE, null);
    }

    private void initModule() {
        ModuleManager.Companion.instance().register(ModuleManager.HOME, new HomeModule());
        ModuleManager.Companion.instance().register(ModuleManager.LOGIN, new LoginModule());
        ModuleManager.Companion.instance().register(ModuleManager.IPC, new BoyiaIPCModule());
        ModuleManager.Companion.instance().register(ModuleManager.SETTING, new BoyiaSettingModule());
        ModuleManager.Companion.instance().register(ModuleManager.SEARCH, new SearchModule());
    }
}
