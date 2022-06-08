package com.boyia.app.core.api;

import android.os.Bundle;
import android.os.RemoteException;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaSender;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.launch.BoyiaAppInfo;
import com.boyia.app.loader.job.JobScheduler;
import com.boyia.app.core.api.ApiConstants.ApiKeys;
import com.boyia.app.core.api.ApiConstants.ApiNames;

public class ApiImplementation {
    private static final String TAG = "ApiImplementation";
    private IBoyiaSender mSender;

    public ApiImplementation(IBoyiaSender sender) {
        mSender = sender;
    }

    public void sendNotification(String action, BoyiaAppInfo info) {
        Bundle bundle = new Bundle();
        bundle.putString(ApiKeys.NOTIFICATION_ACTION, action);
        bundle.putString(ApiKeys.NOTIFICATION_TITLE, info.mAppName);
        bundle.putString(ApiKeys.NOTIFICATION_ICON, info.mAppCover);
        BoyiaIpcData data =
                new BoyiaIpcData(ApiNames.NOTIFICATION_NAME, bundle);
        sendData(data);
    }

    public void setShare(String key, String value) {
        Bundle bundle = new Bundle();
        bundle.putString(ApiKeys.IPC_SHARE_KEY, key);
        bundle.putString(ApiKeys.IPC_SHARE_VALUE, value);
        BoyiaIpcData data = new BoyiaIpcData(
                ApiNames.LOCAL_SHARE_SET,
                bundle
        );

        sendData(data);
    }

    private void sendData(BoyiaIpcData data) {
        try {
            mSender.sendMessageAsync(data, new IBoyiaIpcCallback() {
                @Override
                public void callback(BoyiaIpcData boyiaIpcData) {
                    BoyiaLog.d(TAG, "BoyiaApp boyiaIpcData = " + boyiaIpcData);
                }

                @Override
                public IpcScheduler scheduler() {
                    return runnable -> JobScheduler.jobScheduler().sendJob(runnable::run);
                }
            });
        } catch (RemoteException e) {
            BoyiaLog.e(TAG, String.format("sendData %s error", data.getMethod()), e);
        }
    }
}
