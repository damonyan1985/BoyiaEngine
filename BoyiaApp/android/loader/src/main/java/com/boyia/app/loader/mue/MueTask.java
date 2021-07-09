package com.boyia.app.loader.mue;

import com.boyia.app.loader.job.IScheduler;

/*
 * MueTask
 * 链式任务包装类
 * Author yanbo.boyia
 * All Copyright reserved
 */
public abstract class MueTask<T> implements IMueTask<T> {
    public static <T> MueTask<T> create(MueTaskOnSubscribe<T> source) {
        return new MueTaskCreate<>(source);
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

    public final MueTask<T> subscribeOn(IScheduler scheduler) {
        return new MueTaskSubscribeOn<>(this, scheduler);
    }

    public final MueTask<T> observeOn(IScheduler scheduler) {
        return new MueTaskObserveOn<>(this, scheduler);
    }
}
