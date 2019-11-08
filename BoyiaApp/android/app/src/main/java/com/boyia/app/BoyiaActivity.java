package com.boyia.app;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.core.BoyiaUIView;
import com.boyia.app.loader.job.JobScheduler;

import android.content.Intent;
import android.os.Bundle;
import android.view.KeyEvent;
import android.app.Activity;
import android.os.Process;

public class BoyiaActivity extends Activity {
	private static final String TAG = BoyiaActivity.class.getSimpleName();
	private boolean mNeedExit = false;

	@Override
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		BoyiaUtils.loadLib();
		setContentView(R.layout.main);
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
		BaseApplication.getInstance().getAppHandler().postDelayed(new Runnable() {
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
}
