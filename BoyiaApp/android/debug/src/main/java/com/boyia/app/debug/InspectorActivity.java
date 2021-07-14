package com.boyia.app.debug;

import android.app.Activity;
import android.os.Bundle;

import com.boyia.app.common.ipc.BoyiaIpcHelper;
import com.boyia.app.common.ipc.IBoyiaSender;

public class InspectorActivity extends Activity {
    private static final String TAG = "InspectorActivity";
    private IBoyiaSender mSender;

    @Override
    public void onCreate(Bundle bundle) {
        super.onCreate(bundle);
        // 如果该Activity是主进程启动的，则可以获取主进程binder
        mSender = getIntent().getParcelableExtra(BoyiaIpcHelper.BOYIA_IPC_SENDER);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
