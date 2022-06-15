package com.boyia.app.shell.permission;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
//import android.support.v4.app.ActivityCompat;
//import android.support.v4.content.ContextCompat;

public class BoyiaPermissions {
    public static final int PHOTO_REQUEST_CODE = 1;
    private static final String[] PHOTO_PERMISSIONS = {
            Manifest.permission.CAMERA,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    public static boolean requestPhotoPermissions(Activity context) {
        for (int i = 0; i < PHOTO_PERMISSIONS.length; i++) {
            if (!requestPermission(context, PHOTO_PERMISSIONS[i])) {
                return false;
            }
        }

        return true;
    }

    public static boolean requestPermission(Activity context, String permission) {
        // 如果还没用授权
        if (PackageManager.PERMISSION_GRANTED == ContextCompat.checkSelfPermission(context, permission)) {
            return true;
        }

        if (ActivityCompat.shouldShowRequestPermissionRationale(context, permission)) {
            // 用户之前拒绝过申请
            // TODO 比如弹出一个窗，通知用户曾经拒绝过
        } else {
            // 申请权限
            ActivityCompat.requestPermissions(context, new String[]{ permission }, PHOTO_REQUEST_CODE);
        }

        return false;
    }
}
