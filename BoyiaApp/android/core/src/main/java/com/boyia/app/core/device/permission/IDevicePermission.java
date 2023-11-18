package com.boyia.app.core.device.permission;

import android.content.Context;

public interface IDevicePermission {
    boolean requestBluetooth(Context context);
    boolean requestLocation(Context context);

    boolean requestCamera(Context context);
}
