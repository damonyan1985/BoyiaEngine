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
    private static final int ACTION_REQUEST_CODE = 1;
    private static final String[] BOYIA_PERMISSIONS = {
            Manifest.permission.CAMERA
    };

    public void requestPermission(Activity context, String permission) {
        // 如果还没用授权
        if (PackageManager.PERMISSION_GRANTED != ContextCompat.checkSelfPermission(context, permission)) {
            if (ActivityCompat.shouldShowRequestPermissionRationale(context, permission)) {
                // 用户之前拒绝过申请
                // TODO 比如弹出一个窗，通知用户曾经拒绝过
            } else {
                // 申请权限
                ActivityCompat.requestPermissions(context, new String[]{ permission }, ACTION_REQUEST_CODE);
            }
        }
    }
}
