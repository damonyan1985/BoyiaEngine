package com.boyia.app;

import com.boyia.app.advert.platform.TTAdSplashManager;
import com.boyia.app.broadcast.BoyiaBroadcast;
import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.BoyiaIpcService;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaSender;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.core.BoyiaCoreJNI;
import com.boyia.app.core.BoyiaUIView;
import com.boyia.app.loader.image.BoyiaImager;
import com.boyia.app.loader.job.JobScheduler;

import android.annotation.TargetApi;
import android.content.ComponentCallbacks2;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.os.RemoteException;
import android.view.KeyEvent;
import android.app.Activity;
import android.os.Process;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;

public class BoyiaActivity extends Activity {
    private static final String TAG = "BoyiaActivity";
    private static final String BOYIA_RECEIVE_ACTION = "com.boyia.app.broadcast";
    private boolean mNeedExit = false;
    private BoyiaBroadcast mBroadcast;
    private FrameLayout mContainer;
    private TTAdSplashManager mSplashManager;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mContainer = new FrameLayout(this);
        setContentView(mContainer);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN);
        // 防止重新布局
        //getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_IN_SCREEN);
        //getWindow().addFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);

        mSplashManager = new TTAdSplashManager();
        mSplashManager.loadSplashAd(this,
                mContainer, () -> startBoyiaUI());
        //BoyiaCoreJNI.initLibrary(() -> setContentView(R.layout.main));
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

    private static void setRootView(Activity activity) {
        ViewGroup parent = (ViewGroup) activity.findViewById(android.R.id.content);
        for (int i = 0, count = parent.getChildCount(); i < count; i++) {
            View childView = parent.getChildAt(i);
            if (childView instanceof ViewGroup) {
                childView.setFitsSystemWindows(true);
                ((ViewGroup) childView).setClipToPadding(true);
            }
        }
    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    private static void transparentStatusBar(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
            activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
            //需要设置这个flag contentView才能延伸到状态栏
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION);
            //状态栏覆盖在contentView上面，设置透明使contentView的背景透出来
            activity.getWindow().setStatusBarColor(0xffed4040);
        } else {
            //让contentView延伸到状态栏并且设置状态栏颜色透明
            activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        }
    }

    /**
     * 设置状态栏全透明
     *
     * @param activity 需要设置的activity
     */
    public static void setTransparent(Activity activity) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT) {
            return;
        }
        transparentStatusBar(activity);
        setRootView(activity);
    }


    private void startBoyiaUI() {

        setTransparent(this);
        //getWindow().getDecorView().setFitsSystemWindows(true);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
        BoyiaCoreJNI.initLibrary(() -> {
            mContainer.removeAllViews();
            FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT
            );
            mContainer.addView(new BoyiaUIView(BoyiaActivity.this), params);
        });
    }
}
