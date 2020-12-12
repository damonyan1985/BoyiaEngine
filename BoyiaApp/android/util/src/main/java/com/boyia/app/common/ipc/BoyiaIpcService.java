package com.boyia.app.common.ipc;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import com.boyia.app.common.utils.BoyiaLog;

public class BoyiaIpcService extends Service {
    private static final String TAG = "BoyiaIpcService";
    private static IBoyiaSender sBoyiaSenderDependency;

    private IBoyiaIpcSender.BoyiaSenderStub mBinder;

    public static void setSenderDependency(IBoyiaSender sender) {
        sBoyiaSenderDependency = sender;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        BoyiaLog.i(TAG, "BoyiaIpcService start");
        mBinder = BoyiaIpcHelper.createSenderStub(sBoyiaSenderDependency);
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

    public interface BoyiaIpcBindCallback {
        void callback(IBoyiaSender binder);
    }
}
