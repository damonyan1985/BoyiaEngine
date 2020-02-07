package com.boyia.app.broadcast;

import android.content.Context;
import android.content.Intent;
import android.content.BroadcastReceiver;

public class BoyiaBroadcast extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        abortBroadcast();
    }
}
