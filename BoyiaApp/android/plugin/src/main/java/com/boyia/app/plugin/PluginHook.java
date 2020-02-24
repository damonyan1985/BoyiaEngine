package com.boyia.app.plugin;

import android.app.Instrumentation;
import android.content.Context;
import android.os.Build;

import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

public class PluginHook {
    public static final String TARGET_INTENT = "target_intent";

    public static void hookAMS() throws Exception {
        Object singleton;
        if (Build.VERSION.SDK_INT >= 26) {
            Class<?> clazz = Class.forName("android.app.ActivityManager");
            singleton = ReflectUtil.getField(clazz, null, "IActivityManagerSingleton");
        } else {
            Class<?> activityManagerNativeClass = Class.forName("android.app.ActivityManagerNative");
            singleton = ReflectUtil.getField(activityManagerNativeClass, null, "gDefault");
        }
        Class<?> singletonClass = Class.forName("android.util.Singleton");
        Method getMethod = singletonClass.getMethod("get");
        Object iActivityManager = getMethod.invoke(singleton);
        Class<?> iActivityManagerClass = Class.forName("android.app.IActivityManager");
        Object proxy = Proxy.newProxyInstance(Thread.currentThread().getContextClassLoader(), new Class[]{iActivityManagerClass}, new IActivityManagerProxy(iActivityManager));
        ReflectUtil.setField(singletonClass, singleton, "mInstance", proxy);
    }

    public static void hookInstrumentation(Context context) throws Exception {
        Class<?> contextImplClass = Class.forName("android.app.ContextImpl");
        Object activityThread = ReflectUtil.getField(contextImplClass, context, "mMainThread");
        Class<?> activityThreadClass = Class.forName("android.app.ActivityThread");
        Object mInstrumentation = ReflectUtil.getField(activityThreadClass, activityThread, "mInstrumentation");

        ReflectUtil.setField(activityThreadClass, activityThread, "mInstrumentation", new InstrumentationProxy((Instrumentation) mInstrumentation, context.getPackageManager()));
    }
}
