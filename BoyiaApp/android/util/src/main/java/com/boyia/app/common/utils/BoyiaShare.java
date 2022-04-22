package com.boyia.app.common.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

import com.boyia.app.common.BaseApplication;

// 线程安全非进程安全，子进程需要读取share，必须使用ipc调用主进程功能
public class BoyiaShare {
    private SharedPreferences mShare;

    private static class BoyiaShareHolder {
        public static final BoyiaShare INSTANCE = new BoyiaShare();
    }

    private BoyiaShare() {
        Context context = BaseApplication.getInstance();
        mShare = context.getSharedPreferences(context.getPackageName(),
                Context.MODE_PRIVATE);
    }

    private static Editor editor() {
        return BoyiaShareHolder.INSTANCE.mShare.edit();
    }

    private static SharedPreferences share() {
        return BoyiaShareHolder.INSTANCE.mShare;
    }

    public static void put(String key, int value) {
        Editor editor = editor();
        editor.putInt(key, value);
        editor.commit();
    }

    public static void put(String key, float value) {
        Editor editor = editor();
        editor.putFloat(key, value);
        editor.commit();
    }

    public static void put(String key, long value) {
        Editor editor = editor();
        editor.putLong(key, value);
        editor.commit();
    }

    public static void put(String key, String value) {
        Editor editor = editor();
        editor.putString(key, value);
        editor.commit();
    }

    public static void put(String key, boolean value) {
        Editor editor = editor();
        editor.putBoolean(key, value);
        editor.commit();
    }

    public static int get(String key, int defValue) {
        return share().getInt(key, defValue);
    }

    public static float get(String key, float defValue) {
        return share().getFloat(key, defValue);
    }

    public static long get(String key, long defValue) {
        return share().getLong(key, defValue);
    }

    public static String get(String key, String defValue) {
        return share().getString(key, defValue);
    }

    public static boolean get(String key, boolean defValue) {
        return share().getBoolean(key, defValue);
    }

    public static void clear() {
        Editor editor = editor();
        editor.clear();
        editor.commit();
    }

    public static void remove(String key) {
        Editor editor = editor();
        editor.remove(key);
        editor.commit();
    }

    public boolean contains(String key) {
        return share().contains(key);
    }
}
