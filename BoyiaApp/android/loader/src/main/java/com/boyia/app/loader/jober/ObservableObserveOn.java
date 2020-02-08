package com.boyia.app.loader.jober;

import com.boyia.app.loader.job.Scheduler;

public class ObservableObserveOn<T> extends Observable<T> {
    private final ObservableSource<T> mSource;
    private final Scheduler mScheduler;

    public ObservableObserveOn(ObservableSource<T> source, Scheduler scheduler) {
        mSource = source;
        mScheduler = scheduler;
    }

    @Override
    void subscribeActual(Subscriber<? super T> observer) {
        mSource.subscribe(new Subscriber<T>() {
            @Override
            public void onNext(T t) {
                if (mScheduler != null) {
                    mScheduler.sendJob(() -> observer.onNext(t));
                } else {
                    observer.onNext(t);
                }
            }

            @Override
            public void onError(Throwable e) {
                if (mScheduler != null) {
                    mScheduler.sendJob(() -> observer.onError(e));
                } else {
                    observer.onError(e);
                }
            }

            @Override
            public void onComplete() {
                if (mScheduler != null) {
                    mScheduler.sendJob(() -> observer.onComplete());
                } else {
                    observer.onComplete();
                }
            }
        });
    }
}
