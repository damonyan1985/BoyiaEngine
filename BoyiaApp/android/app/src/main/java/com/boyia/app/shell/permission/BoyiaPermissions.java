package com.boyia.app.shell.permission;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.util.HashMap;
import java.util.Map;

public class BoyiaPermissions {
    // 通知
    public static final String[] NOTIFICATION_PERMISSIONS = {
            "android.permission.POST_NOTIFICATIONS"
    };

    public static final String[] NOTIFICATION_SERVICES_PERMISSIONS = {
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.ACCESS_FINE_LOCATION,
            Manifest.permission.ACCESS_BACKGROUND_LOCATION
    };

    private static boolean checkPermissions(Activity context, String[] permissions) {
        for (String permission : permissions) {
            // 如果没有授权，则返回false
            if (ContextCompat.checkSelfPermission(context, permission) != PackageManager.PERMISSION_GRANTED) {
                if (ActivityCompat.shouldShowRequestPermissionRationale(context, permission)) {
                    // 用户之前拒绝过申请
                    // TODO 比如弹出一个窗，通知用户曾经拒绝过，跳转到设置中设置
                }
                return false;
            }
        }
        return true;
    }

    public static boolean requestPermission(Activity context, String[] permissions, int requestCode) {
        // 如果还没用授权
        if (checkPermissions(context, permissions)) {
            return true;
        }

        ActivityCompat.requestPermissions(context, permissions, requestCode);
        return false;
    }
}
