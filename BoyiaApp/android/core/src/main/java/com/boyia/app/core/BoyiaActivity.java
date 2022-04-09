package com.boyia.app.core;

import android.annotation.TargetApi;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.IntentService;
import android.content.Context;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.FrameLayout;

// Activity持有的mToken是一个IBinder，在C++底层体现就是一个BpBinder(远程服务代理)
// 主要使用来与AMS进行通信的，AMS的任务栈中持有的ActivitRecord与Activity一一对应
// ActivitRecord中持有的appToken为IApplicationToken.Stub, 在C++底层体现就是一个BBinder(服务)

// 每个Window都包含了一个ViewRootImpl，ViewRootImpl的成员mWindow是一个W类型的对象，
// W类型是一个BBinder(Stub服务端)，是一个应用与WMS进行通信的token

// WindowManager.addView相当于是添加了一个窗口
public class BoyiaActivity extends Activity {
    private FrameLayout mContainer;
    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        initBoyiaView();
    }

    protected void initBoyiaView() {
        initContainer();
        startBoyiaUI();
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
            mContainer.addView(new BoyiaView(this), params);
        });
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
        ViewGroup parent = (ViewGroup) activity.findViewById(android.R.id.content);
        for (int i = 0, count = parent.getChildCount(); i < count; i++) {
            View childView = parent.getChildAt(i);
            if (childView instanceof ViewGroup) {
                childView.setFitsSystemWindows(true);
                ((ViewGroup) childView).setClipToPadding(true);
            }
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
