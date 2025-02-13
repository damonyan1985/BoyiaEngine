package com.boyia.app.core.api;

import android.os.Bundle;
import android.os.RemoteException;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaIpcSender;
import com.boyia.app.core.api.ApiHandler.ApiHandlerCallback;

import java.util.HashMap;
import java.util.Map;

/**
 * 接受来自宿主进程的消息，可用于回调
 */
public class ApiAsyncCallbackBinder extends IBoyiaIpcSender.BoyiaSenderStub {
    private final Map<Long, ApiHandlerCallback> mApiCallback = new HashMap<>();

    /**
     * 接受后直接返回给Boyia引擎处理
     * @param message
     * @return
     * @throws RemoteException
     */
    @Override
    public BoyiaIpcData sendMessageSync(BoyiaIpcData message) throws RemoteException {
        Bundle bundle = message.getParams();
        Long callbackID = bundle.getLong(ApiConstants.ApiKeys.CALLBACK_ID);
        ApiHandlerCallback callback = mApiCallback.get(callbackID);
        if (callback != null) {
            callback.callback(bundle.getString(ApiConstants.ApiKeys.CALLBACK_ARGS));
        }

        return null;
    }

    @Override
    public void sendMessageAsync(BoyiaIpcData message, IBoyiaIpcCallback callback) throws RemoteException {

    }

    public void registerApiCallback(Long callbackID, ApiHandlerCallback callback) {
        mApiCallback.put(callbackID, callback);
    }
}
