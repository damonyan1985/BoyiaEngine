package com.boyia.app.common.plugin;

import android.content.Context;
import android.content.res.Resources;

import com.boyia.app.common.utils.BoyiaLog;

public class BoyiaPluginResources extends Resources {
    private static final String TAG = "BoyiaPluginResources";
    private Resources hostResources;
    private Resources injectResources;

    public static Resources createPluginResources(Resources hostResources, Context context, String apkPath) {
        return new BoyiaPluginResources(hostResources, BoyiaPluginUtil.getResources(context, apkPath));
    }

    public BoyiaPluginResources(Resources hostResources, Resources injectResources) {
        super(injectResources.getAssets(), injectResources.getDisplayMetrics(), injectResources.getConfiguration());
        this.hostResources = hostResources;
        this.injectResources = injectResources;
    }

    @Override
    public String getString(int id, Object... formatArgs) throws NotFoundException {
        try {
            return injectResources.getString(id, formatArgs);
        } catch (NotFoundException e) {
            BoyiaLog.e(TAG, "resource not found id = " + id, e);
            return hostResources.getString(id, formatArgs);
        }
    }
}
