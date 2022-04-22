package com.boyia.app.shell;

//import com.boyia.app.advert.platform.TTAdSplashManager;
import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaIpcSender;
import com.boyia.app.core.BoyiaBridge;
import com.boyia.app.loader.mue.MainScheduler;
import com.boyia.app.shell.broadcast.BoyiaBroadcast;
import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.core.BoyiaActivity;
import com.boyia.app.core.BoyiaCoreJNI;
import com.boyia.app.loader.image.BoyiaImager;
import com.boyia.app.loader.job.JobScheduler;
import com.umeng.analytics.MobclickAgent;

import android.content.ComponentCallbacks2;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.KeyEvent;
import android.os.Process;

import java.util.Collections;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.locks.Lock;

public class BoyiaMainActivity extends BoyiaActivity {
    private static final String TAG = "BoyiaMainActivity";
    private static final String BOYIA_RECEIVE_ACTION = "com.boyia.app.broadcast";
    private boolean mNeedExit = false;
    private BoyiaBroadcast mBroadcast;
    //private TTAdSplashManager mSplashManager;
    private IBinder mBinder;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
    }

    @Override
    protected void initBoyiaView() {
        initContainer();
        // mSplashManager = new TTAdSplashManager();
        // mSplashManager.loadSplashAd(this,
        //         getContainer(), () -> startBoyiaUI());
        startBoyiaUI();
        initBroadcast();
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
        MobclickAgent.onPause(this);
        MobclickAgent.onEvent(this, "onPause");
    }

    @Override
    protected void onResume() {
        super.onResume();
        MobclickAgent.onResume(this);
        MobclickAgent.onEvent(this, "onResume");
    }

    @Override
    public void onDestroy() {
        JobScheduler.jobScheduler().stopAllThread();
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
            BoyiaBridge.showToast("再按一次退出程序");
//            BaseApplication.getInstance().getAppHandler().postDelayed(() -> {
//                        mNeedExit = false;
//                    }
//                    , 3000);

            MainScheduler.mainScheduler().sendJobDelay(() -> {
                mNeedExit = false;
            }, 3000);
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
        BoyiaBridge.showToast(intent.getAction());
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
