package com.boyia.app.shell.permission;

import android.app.Activity;
import android.content.Context;

import com.boyia.app.core.device.permission.IDevicePermission;

public class BoyiaDevicePermission implements IDevicePermission {
    @Override
    public boolean requestPermissions(Activity context, String[] permissions, int requestCode) {
        return BoyiaPermissions.requestPermission(context, permissions, requestCode);
    }
}
