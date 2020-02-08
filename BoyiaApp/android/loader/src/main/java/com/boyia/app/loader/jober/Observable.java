package com.boyia.app.loader.jober;

import com.boyia.app.loader.job.Scheduler;

public abstract class Observable<T> implements ObservableSource<T> {
    public static <T> Observable<T> create(ObservableOnSubscribe<T> source) {
        return new ObservableCreate<>(source);
    }

    abstract void subscribeActual(Subscriber<? super T> observer);

    @Override
    public void subscribe(Subscriber<? super T> observer) {
        subscribeActual(observer);
    }

    public void subscribe(ParamAction<? super T> next, ParamAction<? super Throwable> error, Action complete) {
        ActionSubscriber<T> observer = new ActionSubscriber<>(next, error, complete);
        subscribe(observer);
    }

    public void subscribe(ParamAction<? super T> next, ParamAction<? super Throwable> error) {
        subscribe(next, error, Functions.EMPTY_ACTION);
    }

    public void subscribe(ParamAction<? super T> next) {
        subscribe(next, Functions.emptyParamAction(), Functions.EMPTY_ACTION);
    }

    public void subscribe(Action complete) {
        subscribe(Functions.emptyParamAction(), Functions.emptyParamAction(), complete);
    }

    public final Observable<T> subscribeOn(Scheduler scheduler) {
        return new ObservableSubscribeOn<>(this, scheduler);
    }

    public final Observable<T> observeOn(Scheduler scheduler) {
        return new ObservableObserveOn<>(this, scheduler);
    }
}
