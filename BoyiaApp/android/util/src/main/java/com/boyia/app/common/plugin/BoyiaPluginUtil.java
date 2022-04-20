package com.boyia.app.common.plugin;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Resources;

import com.boyia.app.common.utils.BoyiaLog;

public class BoyiaPluginUtil {
    private static final String TAG = "BoyiaPluginUtil";

    public static Resources getResources(Context context, String apkPath) {
        PackageManager pm = context.getPackageManager();
        PackageInfo info = pm.getPackageArchiveInfo(apkPath, PackageManager.GET_ACTIVITIES
                | PackageManager.GET_META_DATA
                | PackageManager.GET_SERVICES
                | PackageManager.GET_PROVIDERS
                | PackageManager.GET_SIGNATURES);

        info.applicationInfo.sourceDir = apkPath;
        info.applicationInfo.publicSourceDir = apkPath;
        try {
            return pm.getResourcesForApplication(info.applicationInfo);
        } catch (PackageManager.NameNotFoundException e) {
            BoyiaLog.e(TAG, "getResources error", e);
        }

        return null;
    }
}
