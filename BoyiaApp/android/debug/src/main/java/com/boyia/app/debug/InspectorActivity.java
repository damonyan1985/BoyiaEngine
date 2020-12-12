package com.boyia.app.debug;

import android.app.Activity;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.RemoteException;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.BoyiaIpcHelper;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaSender;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.job.JobScheduler;


public class InspectorActivity extends Activity {
    private static final String TAG = "InspectorActivity";
    private IBoyiaSender mSender;
    private ServiceConnection mConnection;

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        mConnection = BoyiaIpcHelper.bindService(this, (sender) -> {
            mSender = sender;
            try {
                mSender.sendMessageAsync(new BoyiaIpcData("test", new Bundle()), new IBoyiaIpcCallback() {
                    @Override
                    public void callback(BoyiaIpcData boyiaIpcData) {
                        BoyiaLog.i(TAG, "InspectorActivity sendMessageAsync method=" + boyiaIpcData.getMethod());
                    }

                    @Override
                    public IpcScheduler scheduler() {
                        return (runnable) -> {
                                JobScheduler.getInstance().sendJob(() -> runnable.run());
                            };
                    }
                });
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        });
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mConnection != null) {
            unbindService(mConnection);
        }
    }
}
