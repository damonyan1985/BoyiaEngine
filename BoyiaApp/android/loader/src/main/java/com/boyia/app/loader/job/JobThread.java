package com.boyia.app.loader.job;

import android.os.Process;

/*
 * JobThread
 * @Author Boyia
 * @Time 2018-8-31
 * @Copyright Reserved
 */
public class JobThread extends Thread {
    private boolean mIsWorking;
    private JobScheduler mJobScheduler;
    private boolean mRunning = true;

    public JobThread(JobScheduler shed) {
        mIsWorking = false;
        mJobScheduler = shed;
    }

    public void stopThread() {
        mRunning = false;
    }

    public boolean isWorking() {
        return mIsWorking;
    }

    public void notifyThread() {
        synchronized (this) {
            notify();
        }
    }

    @Override
    public void run() {
        Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);
        while (mRunning) {
            try {
                IJob job = mJobScheduler.pollJob();

                if (job != null) {
                    mIsWorking = true;
                    job.exec();
                    mIsWorking = false;
                } else {
                    if (mJobScheduler.hasNoJob()) {
                        synchronized (this) {
                            try {
                                wait();
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                }
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
    }
}