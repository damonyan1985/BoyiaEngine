package com.boyia.app.loader;

import com.boyia.app.loader.job.IJob;
import com.boyia.app.loader.job.JobQueue;

// SingleThread worker
public class BoyiaWorker extends Thread {
    protected JobQueue mJobQueue;

    private static class BoyiaWaitorHolder {
        static final BoyiaWorker INSTANCE = new BoyiaWorker();
    }

    private BoyiaWorker() {
        synchronized (BoyiaWorker.class) {
            mJobQueue = new JobQueue();
            start();
        }
    }

    public static BoyiaWorker getWorker() {
        return BoyiaWaitorHolder.INSTANCE;
    }

    public void sendJob(IJob task) {
        if (task != null) {
            synchronized (mJobQueue) {
                mJobQueue.addJob(task);
            }

            synchronized (this) {
                notify();
            }
        }
    }

    @Override
    public void run() {
        while (true) {
            IJob task = mJobQueue.poll();
            if (task != null) {
                task.exec();
            } else {
                synchronized (this) {
                    try {
                        wait();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

    }
}
