package com.boyia.app;

import android.content.Context;
import android.support.multidex.MultiDex;

import com.boyia.app.advert.platform.TTAdManagerHolder;
import com.boyia.app.common.BaseApplication;

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
    }
}
