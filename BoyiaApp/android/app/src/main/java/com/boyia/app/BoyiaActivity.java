package com.boyia.app;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.task.JobScheduler;
import com.boyia.app.ui.BoyiaWindow;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.widget.FrameLayout;
import android.os.Process;

public class BoyiaActivity extends BaseActivity {
	private static final String TAG = BoyiaActivity.class.getSimpleName();
	private BoyiaWindow mWindow = null;
	private boolean mNeedExit = false;

	@Override
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		//getWindow().setBackground(getResources().getDrawable(R.drawable.ic_launcher));
		setContentView(new FrameLayout(this));
		initView();
    }

	private void initBoyiaWindow() {
		mWindow = new BoyiaWindow(this);
		mWindow.show();
		mWindow.requestFocus();
	}

	private void initView() {
		BoyiaLog.d("yanbo", "BoyiaAppActivity onCreate");
		BoyiaUtils.loadLib();
		initBoyiaWindow();
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
		if (mWindow != null && mWindow.isVisible()) {
			mWindow.quitUIView();
		}

		JobScheduler.getInstance().stopAllThread();
		super.onDestroy();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		BoyiaLog.d(TAG, "onKeyDown");
		switch (keyCode) {
		case KeyEvent.KEYCODE_BACK:
			if (!mNeedExit) {
				backExit();
				return true;
			} else {
				BoyiaLog.d("yanbo", "BoyiaApplication finished");
				finish();
				Process.killProcess(Process.myPid());
			}
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
		mNeedExit = true;
		BoyiaUtils.showToast("再按一次退出程序");
		getUIHandler().postDelayed(new Runnable() {
			@Override
			public void run() {
				mNeedExit = false;
			}
		}, 3000);
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
	public boolean dispatchKeyEvent(KeyEvent event) {
		return super.dispatchKeyEvent(event);
	}
}
