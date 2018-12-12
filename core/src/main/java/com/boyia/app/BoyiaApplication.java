package com.boyia.app;

import android.app.Application;
import android.content.Context;

public class BoyiaApplication extends Application {
	public static final String BOYIA_DIR = "boyia://";
	public static final String BOYIA_SDK_DIR = "boyiasdk://";
	public static final String DEFUALT_PAGE = "boyia://html/test.html";
	public static final String AUTHOR_NAME = "yanbo";
	
	private static Context sCurrentContext = null;
	private static BoyiaApplication sApplication = null;

	private static boolean sIsLoadJNI = false;
	
	@Override
	public void onCreate() {
		super.onCreate();
		sApplication = this;
	}
	
	public static BoyiaApplication getInstance() {
		return sApplication;
	}
	
	public static void setCurrenContext(Context context) {
		if (!sIsLoadJNI) {
			Thread.setDefaultUncaughtExceptionHandler(new BoyiaCrashHandler());
			//MiniUtils.loadLib(context);
			//MiniUtils.loadLibFromAssets(context);
			sIsLoadJNI = true;
		}
		
		sCurrentContext = context;
	}
	
    public static Context getCurrenContext() {
    	if (sCurrentContext != null) {
    		return sCurrentContext;
    	} else {
    		return sApplication;
    	}
    }
}
