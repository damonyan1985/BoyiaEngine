package com.boyia.app.core.view;

import android.app.Activity;
import android.content.Context;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

public class PlatformViewManager {
    private Map<String, PlatformViewFactory> mFactoryRegistry;
    private Map<String, PlatformViewController> mPlatformViews;
    private WeakReference<Activity> mContext;

    private static class PlatformViewManagerHolder {
        static final PlatformViewManager INSTANCE = new PlatformViewManager();
    }

    public static PlatformViewManager getInstance() {
        return PlatformViewManagerHolder.INSTANCE;
    }

    private PlatformViewManager() {
        mFactoryRegistry = new HashMap<>();
        mPlatformViews = new HashMap<>();
    }

    public PlatformViewFactory getViewFactory(String viewType) {
        return mFactoryRegistry.get(viewType);
    }

    public void registerFactory(String viewType, PlatformViewFactory factory) {
        mFactoryRegistry.put(viewType, factory);
    }

    public void addPlatformView(String viewId, PlatformViewController controller) {
        mPlatformViews.put(viewId, controller);
    }

    public void setContext(Activity activity) {
        mContext = new WeakReference<>(activity);
    }

    public Context getContext() {
        return mContext.get();
    }
}