package com.boyia.app.update;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

public class UpdateApkOperation {
    private IUpdateApkInterface mInterface;
    private ServiceConnection mConnect = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            mInterface = IUpdateApkInterface.Stub.asInterface(iBinder);
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mInterface = null;
        }
    };

    public UpdateApkOperation(Context context) {
        Intent intent = new Intent(context, UpdateApkService.class);
        context.bindService(intent, mConnect, Context.BIND_AUTO_CREATE);
    }

    public void download() {
        if (null != mInterface) {
            try {
                mInterface.download();
            } catch (Exception ex) {
                ex.printStackTrace();
            }

        }
    }
}
