package com.boyia.app.loader.jober;

import android.os.Looper;
import android.os.Handler;
import com.boyia.app.loader.job.IJob;
import com.boyia.app.loader.job.Scheduler;


public class MainScheduler implements Scheduler {
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
}
