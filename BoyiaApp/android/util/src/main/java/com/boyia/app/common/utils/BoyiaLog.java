package com.boyia.app.common.utils;

import android.util.Log;

public class BoyiaLog {
    public static final boolean ENABLE_LOG = true;

    public static void v(String tag, String msg) {
        if (ENABLE_LOG) {
            Log.v(tag, msg);
        }
    }

    public static void i(String tag, String msg) {
        if (ENABLE_LOG) {
            Log.i(tag, msg);
        }
    }

    public static void w(String tag, String msg) {
        if (ENABLE_LOG) {
            Log.w(tag, msg);
        }
    }

    public static void d(String tag, String msg) {
        if (ENABLE_LOG) {
            Log.d(tag, msg);
        }
    }

    public static void e(String tag, String msg) {
        if (ENABLE_LOG) {
            Log.e(tag, msg);
        }
    }

    public static void e(String tag, String msg, Throwable e) {
        if (ENABLE_LOG) {
            Log.e(tag, msg, e);
        }
    }
}
