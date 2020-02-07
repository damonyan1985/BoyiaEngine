package com.boyia.app.loader.job;

import java.util.LinkedList;

/*
 * JobScheduler
 * @Author Boyia
 * @Time 2018-8-31
 * @Copyright Reserved
 */
public class JobScheduler implements Scheduler {
    // Default size of threads
    public static final int DEFAULT_SCHEDULER_THREAD_SIZE = 3;
    // Execute cell implement with thread
    private LinkedList<JobThread> mThreads;
    protected JobQueue mJobQueue;

    private static class JobSchedulerHolder {
        static final JobScheduler INSTANCE = new JobScheduler();
    }

    public static JobScheduler getInstance() {
        return JobSchedulerHolder.INSTANCE;
    }

    public boolean hasNoJob() {
        return mJobQueue.isQueueEmpty();
    }

    private JobScheduler() {
        mThreads = new LinkedList<>();
        mJobQueue = new JobQueue();
    }

    @Override
    public void sendJob(IJob job) {
        synchronized (mJobQueue) {
            if (mJobQueue != null) {
                mJobQueue.addJob(job);
            }
        }

        synchronized (mThreads) {
            for (JobThread thread : mThreads) {
                if (!thread.isWorking()) {
                    thread.notifyThread();
                    return;
                }
            }
        }

        if (mThreads.size() < DEFAULT_SCHEDULER_THREAD_SIZE) {
            JobThread thread = new JobThread(this);
            synchronized (mThreads) {
                mThreads.add(thread);
            }
            thread.start();
        }
    }

    protected IJob pollJob() {
        IJob job;
        synchronized (mJobQueue) {
            job = mJobQueue.poll();
        }

        return job;
    }

    public void stopAllThread() {
        synchronized (mThreads) {
            for (JobThread thread : mThreads) {
                thread.stopThread();
            }
        }
    }
}
