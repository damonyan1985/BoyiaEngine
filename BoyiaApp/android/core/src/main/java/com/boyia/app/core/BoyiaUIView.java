package com.boyia.app.core;

//import android.graphics.PixelFormat;

import android.content.Context;
import android.graphics.PixelFormat;
import android.util.AttributeSet;
import android.view.GestureDetector;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import com.boyia.app.core.input.BoyiaInputConnection;
import com.boyia.app.core.input.BoyiaInputManager;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.view.PlatformViewManager;
import com.boyia.app.core.view.TestPlatformViewFactory;

import android.app.Activity;

/*
 * surfaceview和view不同，view是谁后创建谁在上面
 * surfaceview则是谁先创建谁的surface在上面，但是
 * 其view的属性保持不变，使用setZOrderMediaOverlay
 * 为true后可以使这个view的surface保持在其他媒体surface
 * 之上。
 * surfaceview的view属性默认是在surface之上，也就是
 * surfaceview中ondraw绘制的图像会覆盖在surface上
 *
 * BoyiaUIView, 只是为Opengl提供一个surface容器，具体
 * Opengl初始化，都是在C++，glcontext中进行，所有
 * 与Opengl实现的相关内容均在java层不可见
 * @Author Yan bo
 * @Time 2018-9-1
 * @Copyright Reserved
 * @Descrption OpenGL ES 3.0 Framework Construct By Yanbo
 */
public class BoyiaUIView extends SurfaceView implements SurfaceHolder.Callback {
    protected static final String TAG = "BoyiaUIView";
    private boolean mIsUIViewDistroy = true;
    private SurfaceHolder mHolder = null;
    private BoyiaInputConnection mInputConnect = null;
    private GestureDetector mGestureDetector = null;
    private static BoyiaInputManager mInputManager = null;

    public BoyiaUIView(Context context) {
        this(context, null);
    }

    public BoyiaUIView(Context context, AttributeSet set) {
        super(context, set);
        init(context);
    }

    private void init(Context context) {
        PlatformViewManager.getInstance().setContext(context);
        // 叠在其他surfaceview之上
        BoyiaCoreJNI.nativeInitJNIContext((Activity) context);
        setZOrderOnTop(true);
        //setZOrderMediaOverlay(true);
        getHolder().addCallback(this);
        getHolder().setFormat(PixelFormat.TRANSLUCENT);

        // TODO 为滚动动画做准备
        mGestureDetector = new GestureDetector(getContext(),
                new GestureDetector.OnGestureListener() {

                    @Override
                    public boolean onDown(MotionEvent e) {
                        return false;
                    }

                    @Override
                    public void onShowPress(MotionEvent e) {
                    }

                    @Override
                    public boolean onSingleTapUp(MotionEvent e) {
                        return false;
                    }

                    @Override
                    public boolean onScroll(MotionEvent e1, MotionEvent e2,
                                            float distanceX, float distanceY) {
                        return false;
                    }

                    @Override
                    public void onLongPress(MotionEvent e) {
                    }

                    @Override
                    public boolean onFling(MotionEvent e1, MotionEvent e2,
                                           float velocityX, float velocityY) {
                        return false;
                    }
                });

        initEventListener();

        PlatformViewManager.getInstance().registerFactory("test-view", new TestPlatformViewFactory());
    }

    private void initEventListener() {
        mInputManager = new BoyiaInputManager(this);
        setOnTouchListener((v, event) -> {
            mGestureDetector.onTouchEvent(event);
            onNativeTouch(event);
            return true;
        });

        setOnKeyListener((v, keyCode, event) -> {
                if (event.getAction() == KeyEvent.ACTION_DOWN
                        && event.getKeyCode() == KeyEvent.KEYCODE_DEL) {
                    if (mInputConnect != null) {
                        mInputConnect.deleteCommitText();
                    }
                }
                return false;
            }
        );

        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
    }

    public void setInputText(String text) {
        BoyiaCoreJNI.nativeSetInputText(text, mInputManager.item());
    }

    public void resetCommitText(final String text) {
        if (mInputConnect != null) {
            mInputConnect.resetCommitText(text);
        }
    }

    public static void showKeyboard(long callback, String text) {
        mInputManager.show(callback, text);
    }

    public void onNativeTouch(MotionEvent event) {
        BoyiaCoreJNI.nativeHandleTouchEvent(event.getAction(), (int) event.getX(), (int) event.getY());
    }

    public void onKeyDown(KeyEvent event) {
        if (event.getKeyCode() == KeyEvent.KEYCODE_DEL) {
            //BoyiaUtils.showToast("BoyiaUIView onKeyDown delete");
            if (mInputConnect != null) {
                mInputConnect.deleteCommitText();
            }
        }
        BoyiaCoreJNI.nativeHandleKeyEvent(event.getKeyCode(), 0);
    }

    public View getView() {
        return this;
    }

    public void quitUIView() {
        BoyiaCoreJNI.nativeDistroyUIView();
        mIsUIViewDistroy = true;
    }

    public void initUIView() {
        BoyiaCoreJNI.nativeSetGLSurface(mHolder.getSurface());
        BoyiaCoreJNI.nativeInitUIView(mHolder.getSurfaceFrame().width(),
                mHolder.getSurfaceFrame().height(),
                BoyiaLog.ENABLE_LOG);
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        BoyiaLog.d(TAG, "surfaceCreated");
        mHolder = holder;
        if (mIsUIViewDistroy) {
            BoyiaLog.d(TAG, "initUIView");
            initUIView();
            mIsUIViewDistroy = false;
        } else {
            BoyiaCoreJNI.nativeResetGLSurface(mHolder.getSurface());
            BoyiaLog.d(TAG, "resetGLSurface");
        }
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width,
                               int height) {
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }

    // 这个方法继承自View。把自定义的BaseInputConnection通道传递给InputMethodService
    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        if (mInputConnect == null) {
            mInputConnect = new BoyiaInputConnection(this, false);
        }

        return mInputConnect;
    }
}
