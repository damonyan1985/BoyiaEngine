package com.boyia.app;

import com.boyia.app.base.BaseActivity;
import com.boyia.app.job.JobScheduler;
import com.boyia.app.ui.BoyiaWindow;
import com.boyia.app.upgrade.UpgradeUtil;
import com.boyia.app.utils.BoyiaLog;
import com.boyia.app.utils.BoyiaUtils;
import com.boyia.app.utils.JSONUtil;
import com.boyia.app.utils.BoyiaFileUtil;

import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
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

public class BoyiaActivity extends BaseActivity implements UpgradeUtil.UpgradeListener {
	private static final String TAG = BoyiaActivity.class.getSimpleName();

	private BoyiaWindow mWindow = null;

	private static final int START_MINI_VIRTUAL_CODE = Menu.FIRST + 1;
	private static final int START_MINI_FILESYSTEM = Menu.FIRST + 2;
	private static final int START_MINI_GAME = Menu.FIRST + 3;
	private static final int START_MINI_BROWSER = Menu.FIRST + 4;
	//private BoyiaHomeReceiver mHomeListener = null;
	private boolean mNeedExit = false;
	private TextView mProgressText = null;

	public static class TestClass {
		private String hello;
		private int id;

		public int getId() {
			return id;
		}

		public void setId(int id) {
			this.id = id;
		}

		public String getHello() {
			return hello;
		}

		public void setHello(String hello) {
			this.hello = hello;
		}
	}

	@Override
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		BoyiaApplication.setCurrenContext(this);
		setContentView(new FrameLayout(this));
		getMainView().setBackgroundColor(Color.WHITE);
		
		initView();
		//UpgradeUtil.upgradeAsset(this);
		//UpgradeUtil.upgradeAppFromUrl(UpgradeUtil.ZIP_URL, this);
    }

	private void initBoyiaWindow() {
		mWindow = new BoyiaWindow(this);
		mWindow.show();
		mWindow.requestFocus();
	}

	private void testJsonParse() {
		String json = "{ \"hello\" : \"hello json world\", \"id\" : 1 }";
		TestClass o = JSONUtil.parseJSON(json, TestClass.class);
		BoyiaLog.i("yanbo", "hello=" + o.getHello() + "; id="+o.getId());
	}

	private void initView() {
		getMainView().removeView(mProgressText);
		BoyiaLog.d("yanbo", "BoyiaAppActivity onCreate");
		// getWindow().setFormat(PixelFormat.RGBA_8888);
		//getWindow().setFormat(PixelFormat.TRANSPARENT);
		BoyiaUtils.loadLib();
		// BoyiaTimer.startUpOnTime(60 * 60 * 1000);
		//testHomeReceiver();
		initBoyiaWindow();
		//loadTest();

		Context ctx = (Context)this;
		WeakReference<Context> context = new WeakReference<>(ctx);

		//String text = context.get().getPackageName() + "";

		//split();
		//printSystemInfo();

		//loadTest2();
		BoyiaMultiApplication application = (BoyiaMultiApplication) getApplication();
		application.watcher().watch(this);
	}

	private void resetScreen() {
		WindowManager wm = (WindowManager) BoyiaApplication.getCurrenContext().getSystemService(Context.WINDOW_SERVICE);

		int width = wm.getDefaultDisplay().getWidth();
		int height = wm.getDefaultDisplay().getHeight();

		if (width < height) {
			setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		}
	}
	
	private void printSystemInfo() {
		BoyiaLog.d("SystemInfo", "Product Model: " + android.os.Build.MODEL + ","
				+ android.os.Build.MANUFACTURER + ","
				+ android.os.Build.VERSION.RELEASE);

		String totalsize_str = Formatter.formatFileSize(this,
				BoyiaFileUtil.getSdcardSize());
		String availablesize_str = Formatter.formatFileSize(this,
				BoyiaFileUtil.getSdcardLeftSize());

		String memSize1 = Formatter.formatFileSize(this,
				BoyiaUtils.getTotalMemory());
		String memSize2 = Formatter.formatFileSize(this,
				BoyiaUtils.getSystemAvaialbeMemorySize(this));

		WindowManager wm = getWindowManager();

		int width = wm.getDefaultDisplay().getWidth();
		int height = wm.getDefaultDisplay().getHeight();
		BoyiaLog.d("yanbo", "SDCARD TOTAL SIZE=" + totalsize_str + ";\n"
				+ "LEFT SIZE=" + availablesize_str + ";" + "MEMSIZE1="
				+ memSize1 + ";\n" + "MEMSIZE2=" + memSize2 + ";\n" + "MAC="
				+ BoyiaUtils.getMacAddress(this) + ";\n" 
				+ "SCREENWIDTH=" + width + ";\n"
				+ "SCREENHEIGHT=" + height + ";");

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

	@Override
	public void onUpgradeCompleted() {
		this.runOnUiThread(new Runnable() {

			@Override
			public void run() {
				initView();
			}
		});
	}

	@Override
	public void onUpgradeProgress(int progress) {
		final int progressNum = progress;
		this.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				mProgressText.setText(String.valueOf(progressNum));
			}
		});
	}
}
