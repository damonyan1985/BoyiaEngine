package com.boyia.app.debug;

import android.app.Activity;
import android.os.Bundle;
import android.os.RemoteException;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.BoyiaIpcService;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaSender;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.job.JobScheduler;


public class InspectorActivity extends Activity {
    private static final String TAG = "InspectorActivity";
    private IBoyiaSender mSender;
    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        BoyiaIpcService.bindService(this, (sender) -> {
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
}
