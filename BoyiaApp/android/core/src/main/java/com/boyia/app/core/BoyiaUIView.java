package com.boyia.app.core;

//import android.graphics.PixelFormat;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.input.BoyiaInputConnection;
import com.boyia.app.input.BoyiaInputManager;
import com.boyia.app.common.utils.BoyiaLog;
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
 * @Descrption OpenGL ES 3.0 FrameWork Construct By Yanbo
 */
public class BoyiaUIView extends SurfaceView implements SurfaceHolder.Callback {
	protected static final String TAG = "BoyiaUIView";
	private boolean mIsUIViewDistroy = true;
	private SurfaceHolder mHolder = null;
	private BoyiaInputConnection mInputConnect = null;
	private static BoyiaInputManager mInputManager = null;
	
	public BoyiaUIView(BaseActivity context) {
		super(context);
		nativeInitJNIContext(context);
		init();
		mInputManager = new BoyiaInputManager(this);
	}
	
	private void init() {
		// 叠在其他surfaceview之上
		setZOrderOnTop(true);
		//setZOrderMediaOverlay(true);
		getHolder().addCallback(this);
		getHolder().setFormat(PixelFormat.TRANSLUCENT);
	}
	
	public void setInputText(String text) {
		nativeSetInputText(text, mInputManager.item());
	}
	
	public void resetCommitText(final String text) {
		if (mInputConnect != null) {
			mInputConnect.resetCommitText(text);
		}
	}
	
	public static void showKeyboard(long callback, String text) {
		mInputManager.show(callback, text);
	}
	
	public void onTouchDown(MotionEvent event) {
		nativeHandleTouchEvent(event.getAction(), (int) event.getX(), (int) event.getY());
	}
	
	public void onKeyDown(KeyEvent event) {
		if (event.getKeyCode() == KeyEvent.KEYCODE_DEL) {
			//BoyiaUtils.showToast("BoyiaUIView onKeyDown delete");
			if (mInputConnect != null) {
				mInputConnect.deleteCommitText();
			}
		}
		nativeHandleKeyEvent(event.getKeyCode(), 0);
	}
	
	public View getView() {
		return this;
	}
	
	public void quitUIView() {
		nativeDistroyUIView();
		mIsUIViewDistroy = true;
	}
	
	public void initUIView() {
		nativeInitUIView(mHolder.getSurfaceFrame().width(),
				mHolder.getSurfaceFrame().height(),
				BoyiaLog.ENABLE_LOG);
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		BoyiaLog.d(TAG, "surfaceCreated");
		if (mIsUIViewDistroy) {
			BoyiaLog.d(TAG, " sendBoyiaUIViewTask");
			mHolder = holder;

			nativeSetGLSurface(mHolder.getSurface());
			initUIView();
			mIsUIViewDistroy = false;
		} else {
			nativeResetGLSurface(mHolder.getSurface());
			BoyiaLog.d(TAG, " BoyiaUIViewTask DRAW");
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

	// Native Method Define
    public static native void nativeInitUIView(int width, int height, boolean isDebug);

    public static native void nativeOnDataReceive(byte[] data, int length, long callback);

    public static native void nativeOnDataFinished(long callback);

    public static native void nativeHandleKeyEvent(int keyCode, int isDown);

    public static native void nativeDistroyUIView();

    public static native void nativeImageLoaded(long item);

    public static native void nativeHandleTouchEvent(int type, int x, int y);

    public static native void nativeOnLoadError(String error, long callback);

    public static native void nativeInitJNIContext(Activity context);

    public static native void nativeSetInputText(String text, long item);

    public static native void nativeVideoTextureUpdate(long item);

    public static native void nativeSetGLSurface(Surface surface);

    public static native void nativeResetGLSurface(Surface surface);

    public static native void nativeOnKeyboardShow(long item, int keyboardHeight);

	public static native void nativeOnKeyboardHide(long item, int keyboardHeight);
}
