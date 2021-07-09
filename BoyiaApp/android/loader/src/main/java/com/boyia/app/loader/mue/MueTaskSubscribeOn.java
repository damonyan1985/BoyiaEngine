package com.boyia.app.loader.mue;

import com.boyia.app.loader.job.IScheduler;

/*
 * MueTaskSubscribeOn
 * Author yanbo.boyia
 * All Copyright reserved
 * 发布所在的任务
 */
public class MueTaskSubscribeOn<T> extends MueTask<T> {
    private final IMueTask<T> mSource;
    private final IScheduler mScheduler;

    public MueTaskSubscribeOn(IMueTask<T> source, IScheduler scheduler) {
        mSource = source;
        mScheduler = scheduler;
    }

    // 利用传递进来的schedule执行任务
    @Override
    void subscribeActual(Subscriber<? super T> observer) {
        mScheduler.sendJob(() -> mSource.subscribe(observer));
    }
}
