package com.boyia.app.core.device.permission;

import android.app.Activity;

public class DevicePermissionWrapper implements IDevicePermission {
    private IDevicePermission mDevicePermissionImpl;

    public DevicePermissionWrapper(IDevicePermission permission) {
        mDevicePermissionImpl = permission;
    }

    @Override
    public boolean requestPermissions(Activity context, String[] permissions, int requestCode) {
        return mDevicePermissionImpl.requestPermissions(context, permissions, requestCode);
    }
}
