package com.boyia.app.common;

import android.app.Application;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;

public class BoyiaApplication extends Application {
	public static final String BOYIA_DIR = "boyia://";
	public static final String BOYIA_SDK_DIR = "boyiasdk://";
	public static final String DEFUALT_PAGE = "boyia://html/test.html";
	public static final String AUTHOR_NAME = "yanbo";
	
	private static Context sCurrentContext = null;
	private static BoyiaApplication sApplication = null;
	private HandlerThread mAppThread = null;
	private Handler mHandler = null;
	
	@Override
	public void onCreate() {
		super.onCreate();
		sApplication = this;
		mAppThread = new HandlerThread("app_thread");
		mAppThread.start();
		mHandler = new Handler(mAppThread.getLooper());
		Thread.setDefaultUncaughtExceptionHandler(new BoyiaCrashHandler());
	}
	
	public static BoyiaApplication getInstance() {
		return sApplication;
	}
	
	public static void setCurrenContext(Context context) {
		sCurrentContext = context;
	}
	
    public static Context getCurrenContext() {
    	if (sCurrentContext != null) {
    		return sCurrentContext;
    	} else {
    		return sApplication;
    	}
    }

    public Handler getAppHandler() {
		return mHandler;
	}
}
