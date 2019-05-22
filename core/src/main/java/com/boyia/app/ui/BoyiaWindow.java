package com.boyia.app.ui;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.common.base.BaseWindow;
import com.boyia.app.core.BoyiaUIView;
import com.boyia.app.common.utils.BoyiaLog;

import android.view.KeyEvent;
import android.view.MotionEvent;

public class BoyiaWindow extends BaseWindow {
	private BoyiaUIView mUIView = null;
	public BoyiaWindow(BaseActivity context) {
		super(context);
	}

	@Override
	public void onCreate() {
		super.onCreate();
		mUIView = new BoyiaUIView(getActivity());
		setContentView(mUIView.getView());
	}

	public void setBackgroundColor(int color) {
		mUIView.setBackgroundColor(color);
	}

	public void quitUIView() {
		mUIView.quitUIView();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		BoyiaLog.d("onKeyDown", "test onKeyDown");
		switch (keyCode) {
//		case KeyEvent.KEYCODE_BACK:
//			hide();
//			break;
		default:
			mUIView.onKeyDown(event);
			break;
		}

		return false;
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		mUIView.onTouchDown(event);
		return true;
	}

	@Override
	public void show() {
		super.show();
		BoyiaLog.d("yanbo", "BoyiaBrowser Name="
				+ this.getClass().getSimpleName());
	}
}
