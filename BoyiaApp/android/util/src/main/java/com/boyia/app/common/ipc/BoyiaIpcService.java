package com.boyia.app.common.ipc;

import android.app.Service;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;

import com.boyia.app.common.utils.BoyiaLog;

public class BoyiaIpcService extends Service {
    private static final String TAG = "BoyiaIpcService";
    @Override
    public void onCreate() {
        super.onCreate();
        BoyiaLog.i(TAG, "BoyiaIpcService start");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        BoyiaLog.i(TAG, "BoyiaIpcService bind intent=" +
                (intent != null ? intent.toString() : null));
        return mBinder;
    }

    private IBoyiaIpcSender.BoyiaSenderStub mBinder = new IBoyiaIpcSender.BoyiaSenderStub() {
        @Override
        public BoyiaIpcData sendMessageSync(BoyiaIpcData message) throws RemoteException {
            BoyiaLog.i(TAG, "sendMessageSync method=" + message.getMethod());
            return new BoyiaIpcData("method", new Bundle());
        }

        @Override
        public void sendMessageAsync(BoyiaIpcData message, IBoyiaIpcCallback callback) throws RemoteException {
            BoyiaLog.i(TAG, "sendMessageSync method=" + message.getMethod());
            callback.callback(new BoyiaIpcData("method", new Bundle()));
        }
    };

    public interface BoyiaIpcBindCallback {
        void callback(IBoyiaSender binder);
    }

    /**
     * 客户端调用，获取IBoyiaSender对象
     * @param context
     * @param callback
     */
    public static void bindService(Context context, BoyiaIpcBindCallback callback) {
        context.bindService(new Intent(context, BoyiaIpcService.class), new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder binder) {
                callback.callback(IBoyiaIpcSender.BoyiaSenderStub.asInterface(binder));
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
            }
        }, Context.BIND_AUTO_CREATE);
    }
}
