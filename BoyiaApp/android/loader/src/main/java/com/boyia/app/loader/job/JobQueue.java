package com.boyia.app.loader.job;

import java.util.concurrent.LinkedBlockingQueue;

/*
 * JobQueue
 * @Author Boyia
 * @Time 2018-8-31
 * @Copyright Reserved
 */
public class JobQueue {
    private LinkedBlockingQueue<IJob> mQueue;

    public JobQueue() {
        mQueue = new LinkedBlockingQueue<>();
    }

    public boolean addJob(IJob job) {
        return mQueue.offer(job);
    }

    public IJob poll() {
        return mQueue.peek() != null ? mQueue.poll() : null;
    }

    public boolean isQueueEmpty() {
        return mQueue.isEmpty();
    }

    public void clear() {
        mQueue.clear();
    }
}