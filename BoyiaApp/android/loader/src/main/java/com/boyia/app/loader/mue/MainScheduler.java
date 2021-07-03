package com.boyia.app.loader.mue;

import android.os.Looper;
import android.os.Handler;
import com.boyia.app.loader.job.IJob;
import com.boyia.app.loader.job.IScheduler;

/*
 * MainScheduler
 * Author yanbo.boyia
 * All Copyright reserved
 */
public class MainScheduler implements IScheduler {
    private Handler mHandler;

    private static class MainSchedulerHolder {
        static final MainScheduler INSTANCE = new MainScheduler();
    }

    public static MainScheduler mainScheduler() {
        return MainSchedulerHolder.INSTANCE;
    }

    private MainScheduler() {
        mHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    public void sendJob(IJob job) {
        mHandler.post(() -> job.exec());
    }

    // 延时提交job
    public void sendJobDelay(IJob job, long delayMillis) {
        mHandler.postDelayed(() -> job.exec(), delayMillis);
    }
}
