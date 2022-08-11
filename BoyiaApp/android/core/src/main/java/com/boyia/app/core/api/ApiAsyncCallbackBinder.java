package com.boyia.app.core.api;

import android.os.RemoteException;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaIpcSender;

public class ApiAsyncCallbackBinder extends IBoyiaIpcSender.BoyiaSenderStub {
    @Override
    public BoyiaIpcData sendMessageSync(BoyiaIpcData message) throws RemoteException {
        return null;
    }

    @Override
    public void sendMessageAsync(BoyiaIpcData message, IBoyiaIpcCallback callback) throws RemoteException {

    }
}
