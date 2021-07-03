package com.boyia.app.loader.mue;

import com.boyia.app.loader.job.IScheduler;

/*
 * ObservableSubscribeOn
 * Author yanbo.boyia
 * All Copyright reserved
 */
public class ObservableSubscribeOn<T> extends Observable<T> {
    private final ObservableSource<T> mSource;
    private final IScheduler mScheduler;

    public ObservableSubscribeOn(ObservableSource<T> source, IScheduler scheduler) {
        mSource = source;
        mScheduler = scheduler;
    }

    @Override
    void subscribeActual(Subscriber<? super T> observer) {
        mScheduler.sendJob(() -> mSource.subscribe(observer));
    }
}
