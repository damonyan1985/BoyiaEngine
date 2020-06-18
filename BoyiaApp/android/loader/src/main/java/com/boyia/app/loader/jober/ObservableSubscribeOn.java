package com.boyia.app.loader.jober;

import com.boyia.app.loader.job.Scheduler;

/*
 * ObservableSubscribeOn
 * Author yanbo.boyia
 * All Copyright reserved
 */
public class ObservableSubscribeOn<T> extends Observable<T> {
    private final ObservableSource<T> mSource;
    private final Scheduler mScheduler;

    public ObservableSubscribeOn(ObservableSource<T> source, Scheduler scheduler) {
        mSource = source;
        mScheduler = scheduler;
    }

    @Override
    void subscribeActual(Subscriber<? super T> observer) {
        mScheduler.sendJob(() -> mSource.subscribe(observer));
    }
}
