package com.boyia.app.core.device.bluetooth;

import android.Manifest;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;

public class BoyiaBluetooth {
    // Adapter如果为空，则表示不支持蓝牙
    private BluetoothAdapter mAdapter;

    public BoyiaBluetooth(Context context) {
        final BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mAdapter = BluetoothAdapter.getDefaultAdapter();
        } else {
            mAdapter = bluetoothManager.getAdapter();
        }
    }
    @SuppressLint("MissingPermission")
    public void openBluetooth(Context context) {
        if (mAdapter.isEnabled()) {
            return;
        }
        Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
        ((Activity) context).startActivityForResult(intent, 0);
    }
}
