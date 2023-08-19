package com.boyia.app.shell.permission;

import android.content.Context;

import com.boyia.app.core.device.permission.IDevicePermission;

public class BoyiaDevicePermission implements IDevicePermission {
    @Override
    public boolean requestBluetooth(Context context) {
        return false;
    }

    @Override
    public boolean requestLocation(Context context) {
        return false;
    }
}
