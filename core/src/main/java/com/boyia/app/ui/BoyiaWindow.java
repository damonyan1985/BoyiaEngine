package com.boyia.app.ui;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.common.base.BaseWindow;
import com.boyia.app.core.BoyiaUIView;
import com.boyia.app.common.utils.BoyiaLog;

import android.view.KeyEvent;
import android.view.MotionEvent;

public class BoyiaWindow extends BaseWindow {
	private BoyiaUIView mWebView = null;

	public BoyiaWindow(BaseActivity context) {
		super(context);
	}

	@Override
	public void onCreate() {
		super.onCreate();
		mWebView = new BoyiaUIView(getActivity());
		setContentView(mWebView.getView());
	}

	public void setBackgroundColor(int color) {
		mWebView.setBackgroundColor(color);
	}

	public void loadUrl(String url) {
		mWebView.loadUrl(url);
	}

	public void quitUIView() {
		mWebView.quitUIView();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		BoyiaLog.d("onKeyDown", "test onKeyDown");
		switch (keyCode) {
//		case KeyEvent.KEYCODE_BACK:
//			hide();
//			break;
		default:
			mWebView.onKeyDown(event);
			break;
		}

		return false;
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		mWebView.onTouchDown(event);
		return true;
	}

	@Override
	public void show() {
		super.show();
		BoyiaLog.d("yanbo", "BoyiaBrowser Name="
				+ this.getClass().getSimpleName());
	}
}
