package com.boyia.app;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.task.JobScheduler;
import com.boyia.app.ui.BoyiaWindow;
import com.boyia.app.upgrade.UpgradeUtil;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.common.utils.JSONUtil;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.BoyiaApplication;
import com.squareup.leakcanary.LeakCanary;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.text.format.Formatter;
import android.view.KeyEvent;
import android.view.MenuItem;
import android.view.WindowManager;
import android.view.Menu;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.os.Process;

import java.lang.ref.WeakReference;

public class BoyiaActivity extends BaseActivity {
	private static final String TAG = BoyiaActivity.class.getSimpleName();

	private BoyiaWindow mWindow = null;

	private static final int START_MINI_VIRTUAL_CODE = Menu.FIRST + 1;
	private static final int START_MINI_FILESYSTEM = Menu.FIRST + 2;
	private static final int START_MINI_GAME = Menu.FIRST + 3;
	private static final int START_MINI_BROWSER = Menu.FIRST + 4;
	private boolean mNeedExit = false;

	@Override
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		//getWindow().getDecorView().setBackground(getResources().getDrawable(R.drawable.ic_launcher));
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
	public boolean onCreateOptionsMenu(Menu menu) {
		menu.add(Menu.NONE, START_MINI_VIRTUAL_CODE, 1, "销毁浏览器");
		menu.add(Menu.NONE, START_MINI_FILESYSTEM, 2, "启动文件系统");
		menu.add(Menu.NONE, START_MINI_GAME, 2, "启动游戏");
		menu.add(Menu.NONE, START_MINI_BROWSER, 2, "启动浏览器");
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case START_MINI_FILESYSTEM: {
			break;
		}
		case START_MINI_VIRTUAL_CODE: {
			((BoyiaWindow) mWindow).quitUIView();
			BoyiaUtils.showToast("销毁Window");
		}
			break;
		case START_MINI_BROWSER: {
			mWindow.show();
		}
			break;
		}

		return false;
	}

	@Override
	public void onDestroy() {
		if (mWindow != null && mWindow.isVisible()) {
			((BoyiaWindow) mWindow).quitUIView();
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
	public boolean onContextItemSelected(MenuItem item) {
		BoyiaUtils.showToast("测试全局菜单");
		return super.onContextItemSelected(item);
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
