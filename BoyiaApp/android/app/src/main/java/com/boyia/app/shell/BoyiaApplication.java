package com.boyia.app.shell;

import android.content.Context;
import android.support.multidex.MultiDex;

import com.boyia.app.advert.platform.TTAdManagerHolder;
import com.boyia.app.common.BaseApplication;
import com.umeng.commonsdk.UMConfigure;

public class BoyiaApplication extends BaseApplication {
    @Override
    protected void attachBaseContext(Context base) {
        super.attachBaseContext(base);
        MultiDex.install(base);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        TTAdManagerHolder.init(this);
        UMConfigure.setLogEnabled(true);
        UMConfigure.init(this, null, null,
                UMConfigure.DEVICE_TYPE_PHONE, null);
    }
}
