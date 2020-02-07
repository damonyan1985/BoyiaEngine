package com.boyia.app.loader.jober;

import com.boyia.app.loader.job.Scheduler;

public abstract class Observable<T> implements ObservableSource<T> {
    protected Scheduler mSubscribeOnScheduler;
    protected Scheduler mObserveOnScheduler;

    public static <T> Observable<T> create(ObservableOnSubscribe<T> source) {
        return new ObservableCreate<>(source);
    }

    abstract void subscribeActual(Subscriber<? super T> observer);

    @Override
    public void subscribe(Subscriber<? super T> observer) {
        subscribeActual(observer);
    }

    public final Observable<T> subscribeOn(Scheduler scheduler) {
        mSubscribeOnScheduler = scheduler;
        return this;
    }

    public final Observable<T> observeOn(Scheduler scheduler) {
        mObserveOnScheduler = scheduler;
        return this;
    }
}
