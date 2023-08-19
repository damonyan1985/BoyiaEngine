package com.boyia.app.core.api;

import android.os.RemoteException;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaIpcSender;

/**
 * 接受来自宿主进程的消息，可用于回调
 */
public class ApiAsyncCallbackBinder extends IBoyiaIpcSender.BoyiaSenderStub {
    @Override
    public BoyiaIpcData sendMessageSync(BoyiaIpcData message) throws RemoteException {
        return null;
    }

    @Override
    public void sendMessageAsync(BoyiaIpcData message, IBoyiaIpcCallback callback) throws RemoteException {

    }
}
