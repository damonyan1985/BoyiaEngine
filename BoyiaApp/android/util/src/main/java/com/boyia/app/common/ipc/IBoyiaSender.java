package com.boyia.app.common.ipc;

import android.os.RemoteException;

/**
 * 跨进程通信接口
 */
public interface IBoyiaSender {
    /**
     * 同步发消息
     */
    BoyiaIpcData sendMessageSync(BoyiaIpcData message) throws RemoteException;

    /**
     * 异步发消息
     */
    void sendMessageAsync(BoyiaIpcData message, IBoyiaIpcCallback callback) throws RemoteException;
}
