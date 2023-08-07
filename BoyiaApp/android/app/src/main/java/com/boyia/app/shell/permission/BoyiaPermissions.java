package com.boyia.app.shell.permission;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class BoyiaPermissions {
    /**
     * 摄像头请求
     */
    public static final int CAMERA_REQUEST_CODE = 1;
    /**
     * 蓝牙权限请求
     */
    public static final int BLUETOOTH_REQUEST_CODE = 2;
    /**
     * 存储器权限请求
     */
    public static final int STORYAGE_REQUEST_CODE = 3;

    private static final String[] CAMERA_PERMISSIONS = {
            Manifest.permission.CAMERA,
            Manifest.permission.READ_EXTERNAL_STORAGE,
            Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    // 蓝牙权限
    private static final String[] BLUETOOTH_PERMISSIONS = {
            Manifest.permission.BLUETOOTH_SCAN,
            Manifest.permission.BLUETOOTH_ADVERTISE,
            Manifest.permission.BLUETOOTH_CONNECT
    };

    // 存储权限
    private static final String[] STORAGE_PERMISSIONS = {
            Manifest.permission.WRITE_EXTERNAL_STORAGE,
            Manifest.permission.READ_EXTERNAL_STORAGE
    };

    // 位置权限
    private static final String[] LOCATION_PERMISSIONS = {
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.ACCESS_FINE_LOCATION
    };

    public static boolean requestPhotoPermissions(Activity context) {
        for (int i = 0; i < CAMERA_PERMISSIONS.length; i++) {
            if (!requestPermission(context, CAMERA_PERMISSIONS[i], CAMERA_REQUEST_CODE)) {
                return false;
            }
        }

        return true;
    }

    public static boolean requestBluetoothPermissions(Activity context) {
        for (int i = 0; i < BLUETOOTH_PERMISSIONS.length; i++) {
            if (!requestPermission(context, BLUETOOTH_PERMISSIONS[i], BLUETOOTH_REQUEST_CODE)) {
                return false;
            }
        }

        return true;
    }

    public static boolean requestLocationPermissions(Activity context) {
        for (int i = 0; i < LOCATION_PERMISSIONS.length; i++) {
            if (!requestPermission(context, LOCATION_PERMISSIONS[i], STORYAGE_REQUEST_CODE)) {
                return false;
            }
        }

        return true;
    }

    public static boolean requestPermission(Activity context, String permission, int requestCode) {
        // 如果还没用授权
        if (PackageManager.PERMISSION_GRANTED == ContextCompat.checkSelfPermission(context, permission)) {
            return true;
        }

        if (ActivityCompat.shouldShowRequestPermissionRationale(context, permission)) {
            // 用户之前拒绝过申请
            // TODO 比如弹出一个窗，通知用户曾经拒绝过
        } else {
            // 申请权限
            ActivityCompat.requestPermissions(context, new String[]{ permission }, requestCode);
        }

        return false;
    }
}
