package com.boyia.app.core.device.permission;

import android.content.Context;

public class DevicePermissionWrapper implements IDevicePermission {
    private IDevicePermission mDevicePermissionImpl;

    public DevicePermissionWrapper(IDevicePermission permission) {
        mDevicePermissionImpl = permission;
    }

    @Override
    public boolean requestBluetooth(Context context) {
        return mDevicePermissionImpl.requestBluetooth(context);
    }

    @Override
    public boolean requestLocation(Context context) {
        return mDevicePermissionImpl.requestLocation(context);
    }

    @Override
    public boolean requestCamera(Context context) {
        return mDevicePermissionImpl.requestCamera(context);
    }
}
