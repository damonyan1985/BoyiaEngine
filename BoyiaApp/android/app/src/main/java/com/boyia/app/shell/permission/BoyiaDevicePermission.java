package com.boyia.app.shell.permission;

import android.app.Activity;
import android.content.Context;

import com.boyia.app.core.device.permission.IDevicePermission;

public class BoyiaDevicePermission implements IDevicePermission {
    @Override
    public boolean requestBluetooth(Context context) {
        return BoyiaPermissions.requestBluetoothPermissions((Activity) context);
    }

    @Override
    public boolean requestLocation(Context context) {
        return BoyiaPermissions.requestPhotoPermissions((Activity) context);
    }

    public boolean requestCamera(Context context) {
        return BoyiaPermissions.requestCameraPermissions((Activity) context);
    }
}
