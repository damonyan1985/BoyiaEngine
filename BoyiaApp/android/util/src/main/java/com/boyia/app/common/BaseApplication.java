package com.boyia.app.common;

import android.app.Application;
import android.os.Looper;
import android.os.Handler;
import android.os.HandlerThread;

public class BaseApplication extends Application {
    private static BaseApplication sApplication = null;
    private HandlerThread mAppThread = null;
    private Handler mHandler = null;

    @Override
    public void onCreate() {
        super.onCreate();
        sApplication = this;
        mHandler = new Handler(Looper.getMainLooper());
    }

    public static BaseApplication getInstance() {
        return sApplication;
    }

    public Handler getAppHandler() {
        return mHandler;
    }
}