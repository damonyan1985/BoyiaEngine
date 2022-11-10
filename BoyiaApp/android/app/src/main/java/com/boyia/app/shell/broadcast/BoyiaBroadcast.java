package com.boyia.app.shell.broadcast;

import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;

public class BoyiaBroadcast extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        // 拦截广播
        abortBroadcast();
    }
}
