package com.boyia.app.shell;

import android.content.Context;
import androidx.multidex.MultiDex;

import com.boyia.app.common.BaseApplication;
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
        LeakChecker.install(this);
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
