package com.boyia.app.core;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.IntentService;
import android.content.ComponentCallbacks2;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Looper;
import android.os.Process;
import android.os.RemoteException;
import android.view.KeyEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;

import com.boyia.app.common.ipc.BoyiaIpcData;
import com.boyia.app.common.ipc.IBoyiaIpcCallback;
import com.boyia.app.common.ipc.IBoyiaIpcSender;
import com.boyia.app.common.ipc.IBoyiaSender;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.launch.BoyiaAppInfo;
import com.boyia.app.core.launch.BoyiaAppLauncher;
import com.boyia.app.loader.image.BoyiaImager;
import com.boyia.app.loader.job.JobScheduler;
import com.boyia.app.loader.mue.MainScheduler;

// Activity持有的mToken是一个IBinder，在C++底层体现就是一个BpBinder(远程服务代理)
// 主要使用来与AMS进行通信的，AMS的任务栈中持有的ActivitRecord与Activity一一对应
// ActivitRecord中持有的appToken为IApplicationToken.Stub, 在C++底层体现就是一个BBinder(服务)

// 每个Window都包含了一个ViewRootImpl，ViewRootImpl的成员mWindow是一个W类型的对象，
// W类型是一个BBinder(Stub服务端)，是一个应用与WMS进行通信的token

// WindowManager.addView相当于是添加了一个窗口
// Activity是一个ContextThemeWrapper，从ContextWrapper派生而来
// Activity attach的时候会将ContextImpl传入
public class BoyiaActivity extends Activity {
    private static final String TAG = "BoyiaActivity";
    private FrameLayout mContainer;
    private BoyiaAppInfo mAppInfo;
    private IBoyiaSender mSender;
    private boolean mNeedExit = false;
    private static final int EXIT_DELAY_TIME = 3000;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        fetchAppInfo();
        initBoyiaView();
    }

    protected void initBoyiaView() {
        initContainer();
        startBoyiaUI();
    }

    private void fetchAppInfo() {
        Intent intent = getIntent();
        Bundle bundle = intent.getExtras();
        mAppInfo = bundle.getParcelable(BoyiaAppLauncher.BOYIA_APP_INFO_KEY);
        mSender = IBoyiaIpcSender.BoyiaSenderStub.asInterface(mAppInfo.mHostBinder);
        BoyiaBridge.setIPCSender(mSender);

        justForTest();
    }

    private void justForTest() {
        Bundle b = new Bundle();
        b.putString("ipc_key", "key1");
        b.putString("ipc_value", "value1");
        BoyiaIpcData data = new BoyiaIpcData(
                "local_share_set",
                b
        );
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
            e.printStackTrace();
        }
    }

    protected void initContainer() {
        mContainer = new FrameLayout(this);
        setContentView(mContainer);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN);
    }

    protected ViewGroup getContainer() {
        return mContainer;
    }

    protected void startBoyiaUI() {
        setTransparent(this);
        //getWindow().getDecorView().setFitsSystemWindows(true);
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE);
        // 加载so之后更新UI
        BoyiaCoreJNI.initLibrary(() -> {
            mContainer.removeAllViews();
            FrameLayout.LayoutParams params = new FrameLayout.LayoutParams(
                    ViewGroup.LayoutParams.MATCH_PARENT,
                    ViewGroup.LayoutParams.MATCH_PARENT
            );
            mContainer.addView(new BoyiaView(this, mAppInfo), params);
        });
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        BoyiaLog.d(TAG, "onNewIntent intent action = " + intent.getAction());

        Bundle bundle = intent.getExtras();
        BoyiaAppInfo info = (BoyiaAppInfo) bundle.get(BoyiaAppLauncher.BOYIA_APP_INFO_KEY);
        if (info == null) {
            return;
        }


        if (mAppInfo.mAppId != info.mAppId) {
            // TODO 如果appid不一样，则重新刷新应用
            mAppInfo = info;
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

    private static void setRootView(Activity activity) {
        ViewGroup parent = activity.findViewById(android.R.id.content);
        for (int i = 0, count = parent.getChildCount(); i < count; i++) {
            View childView = parent.getChildAt(i);
            if (childView instanceof ViewGroup) {
                childView.setFitsSystemWindows(true);
                ((ViewGroup) childView).setClipToPadding(true);
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        BoyiaLog.d(TAG, "onKeyDown");
        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                //backExit();
                backToBackground();
                return true;
            case KeyEvent.KEYCODE_DPAD_DOWN:
            case KeyEvent.KEYCODE_DPAD_UP:
            case KeyEvent.KEYCODE_ENTER:
            case KeyEvent.KEYCODE_DPAD_CENTER:
                break;
        }

        return super.onKeyDown(keyCode, event);
    }

    /**
     * 退到后台不销毁
     */
    public void backToBackground() {
        moveTaskToBack(true);
        BoyiaCoreJNI.nativeCacheCode();
    }

    public void backExit() {
        if (mNeedExit) {
            BoyiaLog.d(TAG, "BoyiaApplication finished");
            // 退出程序时将程序内存快照保存在本地
            BoyiaCoreJNI.nativeCacheCode();
            finish();
            Process.killProcess(Process.myPid());
        } else {
            mNeedExit = true;
            BoyiaBridge.showToast("再按一次退出程序");
            MainScheduler.mainScheduler().sendJobDelay(() -> {
                mNeedExit = false;
            }, EXIT_DELAY_TIME);
        }
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);
        BoyiaLog.d(TAG, "onTrimMemory level=" + level);
        if (level >= ComponentCallbacks2.TRIM_MEMORY_MODERATE) {
            BoyiaImager.getInstance().clearMemoryCache();
        }
    }

    // 以下是BoyiaApp子进程启动类, 最多只能启动6个应用进程
    public static class BoyiaAppActivityA extends BoyiaActivity {}

    public static class BoyiaAppActivityB extends BoyiaActivity {}

    public static class BoyiaAppActivityC extends BoyiaActivity {}

    public static class BoyiaAppActivityD extends BoyiaActivity {}

    public static class BoyiaAppActivityE extends BoyiaActivity {}

    public static class BoyiaAppActivityF extends BoyiaActivity {}
}
