package com.boyia.app.shell.update;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaIpcSender;
import com.boyia.app.loader.job.JobScheduler;

import java.util.ArrayList;
import java.util.List;

public class UpdateApkOperation {
    public static final String PARAM_FILE_URL = "file_url";

    public static final String METHOD_DOWNLOAD = "download";
    public static final String METHOD_PAUSE = "pause";
    public static final String METHOD_DELETE = "delete";

    private IBoyiaIpcSender mInterface;
    private List<String> mDownloadUrls = new ArrayList<>();

    private ServiceConnection mConnect = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            mInterface = IBoyiaIpcSender.BoyiaSenderStub.asInterface(iBinder);
            if (mInterface != null && !mDownloadUrls.isEmpty()) {
                for (int i = 0; i < mDownloadUrls.size(); i++) {
                    download(mDownloadUrls.get(i));
                }
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            mInterface = null;
        }
    };

    public UpdateApkOperation(Context context) {
        Intent intent = new Intent(context, UpdateApkService.class);
        context.bindService(intent, mConnect, Context.BIND_AUTO_CREATE);
    }

    public void download(String url) {
        if (null != mInterface) {
            try {
                Bundle bundle = new Bundle();
                bundle.putString(PARAM_FILE_URL, url);
                mInterface.sendMessageAsync(new BoyiaIpcData(METHOD_DOWNLOAD, bundle), new IBoyiaIpcCallback() {
                    @Override
                    public void callback(BoyiaIpcData boyiaIpcData) {
                    }

                    @Override
                    public IpcScheduler scheduler() {
                        return (runnable) -> {
                            JobScheduler.jobScheduler().sendJob(() -> runnable.run());
                        };
                    }
                });
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        } else {
            mDownloadUrls.add(url);
        }
    }
}
