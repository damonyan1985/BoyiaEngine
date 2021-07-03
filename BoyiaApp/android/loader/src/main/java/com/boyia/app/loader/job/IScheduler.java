package com.boyia.app.loader.job;

/*
 * Scheduler
 * Author yanbo.boyia
 * All Copyright reserved
 */
public interface IScheduler {
    // send a job to schedule
    void sendJob(IJob job);
}
