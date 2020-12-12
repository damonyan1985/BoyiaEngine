package com.boyia.app;

import com.boyia.app.broadcast.BoyiaBroadcast;
import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.BoyiaIpcService;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaSender;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.core.BoyiaCoreJNI;
import com.boyia.app.loader.image.BoyiaImager;
import com.boyia.app.loader.job.IJob;
import com.boyia.app.loader.job.JobScheduler;
import com.boyia.app.loader.jober.MainScheduler;
import com.boyia.app.loader.jober.Observable;
import com.boyia.app.loader.jober.Subscriber;

import android.content.ComponentCallbacks2;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.RemoteException;
import android.view.KeyEvent;
import android.app.Activity;
import android.os.Process;

public class BoyiaActivity extends Activity {
    private static final String TAG = BoyiaActivity.class.getSimpleName();
    private static final String BOYIA_RECEIVE_ACTION = "com.boyia.app.broadcast";
    private boolean mNeedExit = false;
    private BoyiaBroadcast mBroadcast;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        BoyiaCoreJNI.initLibrary(() -> setContentView(R.layout.main));
        initBroadcast();
        initService();
    }

    private void initService() {
        BoyiaIpcService.setSenderDependency(new IBoyiaSender(){
            @Override
            public BoyiaIpcData sendMessageSync(BoyiaIpcData boyiaIpcData) throws RemoteException {
                return new BoyiaIpcData("MainActivitySyncMethod", new Bundle());
            }

            @Override
            public void sendMessageAsync(BoyiaIpcData boyiaIpcData, IBoyiaIpcCallback callback) throws RemoteException {
                callback.callback(new BoyiaIpcData("MainActivityAsyncMethod", new Bundle()));
            }
        });

        startService(new Intent(this, BoyiaIpcService.class));
    }

    private void initBroadcast() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(BOYIA_RECEIVE_ACTION);
        mBroadcast = new BoyiaBroadcast();
        registerReceiver(mBroadcast, filter);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onDestroy() {
        JobScheduler.getInstance().stopAllThread();
        super.onDestroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        BoyiaLog.d(TAG, "onKeyDown");
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                backExit();
                break;
            case KeyEvent.KEYCODE_DPAD_DOWN:
            case KeyEvent.KEYCODE_DPAD_UP:
            case KeyEvent.KEYCODE_ENTER:
            case KeyEvent.KEYCODE_DPAD_CENTER:
                break;
        }

        return super.onKeyDown(keyCode, event);
    }

    public void backExit() {
        if (mNeedExit) {
            BoyiaLog.d("yanbo", "BoyiaApplication finished");
            // 退出程序时将程序内存快照保存在本地
            BoyiaCoreJNI.nativeCacheCode();
            finish();
            Process.killProcess(Process.myPid());
        } else {
            mNeedExit = true;
            BoyiaUtils.showToast("再按一次退出程序");
            BaseApplication.getInstance().getAppHandler().postDelayed(() -> {
                        mNeedExit = false;
                    }
                    , 3000);
        }
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        return false;
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
    }

    @Override
    public void onNewIntent(Intent intent) {
        BoyiaUtils.showToast(intent.getAction());
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
        BoyiaLog.d(TAG, "onTrimMemory level=" + level);
        if (level >= ComponentCallbacks2.TRIM_MEMORY_MODERATE) {
            BoyiaImager.getInstance().clearMemoryCache();
        }
    }
}
