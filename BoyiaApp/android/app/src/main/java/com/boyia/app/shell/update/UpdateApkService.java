package com.boyia.app.shell.update;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteException;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaIpcSender;

public class UpdateApkService extends Service {
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mStub;
    }

    public IBoyiaIpcSender.BoyiaSenderStub mStub = new IBoyiaIpcSender.BoyiaSenderStub() {

        @Override
        public BoyiaIpcData sendMessageSync(BoyiaIpcData boyiaIpcData) throws RemoteException {
            return null;
        }

        @Override
        public void sendMessageAsync(BoyiaIpcData boyiaIpcData, IBoyiaIpcCallback iBoyiaIpcCallback) throws RemoteException {
            new Downloader().download(new DownloadData());
        }
    };
}
