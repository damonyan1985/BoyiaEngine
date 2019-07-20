package com.boyia.app.common.utils;

import com.boyia.app.common.BoyiaApplication;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;

public class BoyiaTimer {
	public static final String TIMER_ACTION = "Boyia.app.utils.timer.action";

	private static final int ON_TIME_END = 1;
	private static final int QUIT_TIMER = 2;

	private Handler mHandler = null;
	private HandlerThread mTimerThread = null;
	private static BoyiaTimer sTimer = null;

	public interface TimerCallback {
		void onTimeEnd();
	}

	public static BoyiaTimer getTimer() {
		if (null == sTimer) {
			sTimer = new BoyiaTimer();
		}

		return sTimer;
	}

	private BoyiaTimer() {
		initTimer();
	}

	private void initTimer() {
		mTimerThread = new HandlerThread("boyia_timer");
		mTimerThread.start();
		mHandler = new Handler(mTimerThread.getLooper()) {
			public void handleMessage(Message msg) {
				TimerCallback callback = (TimerCallback) msg.obj;
				switch (msg.what) {
				case ON_TIME_END:
					callback.onTimeEnd();
					break;
				case QUIT_TIMER:
					mTimerThread.quit();
					sTimer = null;
					break;
				}
			}
		};
	}

	public void removeAllTimer() {
		if (mHandler != null) {
			mHandler.removeMessages(ON_TIME_END);
		}
	}
	
	public void stop(TimerCallback callback) {
		if (mHandler != null) {
		    mHandler.removeMessages(ON_TIME_END, callback);
		}
	}

	public void start(TimerCallback callback) {
		start(callback, 0);
	}

	public void start(TimerCallback callback, long delayTime) {
		if (mHandler != null) {
			Message msg = Message.obtain();
			msg.obj = callback;
			msg.what = ON_TIME_END;
			mHandler.sendMessageDelayed(msg, delayTime);
		}
	}

	public void quit() {
		removeAllTimer();
		if (mHandler != null) {
			mHandler.sendEmptyMessage(QUIT_TIMER);
		}
	}

	public static void startUpOnTime(long duration) {
		Intent intent = new Intent();
		intent.setAction(TIMER_ACTION);

		AlarmManager am = (AlarmManager) BoyiaApplication.getCurrenContext()
				.getSystemService(Context.ALARM_SERVICE);
		PendingIntent pi = PendingIntent.getBroadcast(BoyiaApplication.getCurrenContext(), 0, intent, 0);

		am.setRepeating(AlarmManager.RTC_WAKEUP, System.currentTimeMillis(),
				duration, pi);
	}
}
