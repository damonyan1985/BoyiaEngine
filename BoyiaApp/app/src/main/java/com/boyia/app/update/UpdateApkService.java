package com.boyia.app.update;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

public class UpdateApkService extends Service {

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mStub;
    }

    public IUpdateApkInterface.Stub mStub = new IUpdateApkInterface.Stub() {
        @Override
        public void download() throws RemoteException {

        }
    };
}
