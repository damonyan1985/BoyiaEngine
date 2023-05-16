package com.boyia.app.core.device.bluetooth;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.os.Build;

public class BoyiaBluetooth {
    private BluetoothAdapter mAdapter;
    public BoyiaBluetooth(Context context) {
        final BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mAdapter = BluetoothAdapter.getDefaultAdapter();
        } else {
            mAdapter = bluetoothManager.getAdapter();
        }
    }
}
