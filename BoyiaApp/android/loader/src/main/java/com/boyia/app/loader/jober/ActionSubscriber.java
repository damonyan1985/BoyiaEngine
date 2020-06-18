package com.boyia.app.loader.jober;

/*
 * ActionSubscriber
 * Author yanbo.boyia
 * All Copyright reserved
 */
public class ActionSubscriber<T> implements Subscriber<T> {
    private final Action mOnComplete;
    private final ParamAction<? super T> mOnNext;
    private final ParamAction<? super Throwable> mOnError;

    ActionSubscriber(ParamAction<? super T> onNext, ParamAction<? super Throwable> onError, Action onComplete) {
        mOnNext = onNext;
        mOnError = onError;
        mOnComplete = onComplete;
    }

    @Override
    public void onNext(T t) {
        mOnNext.doAction(t);
    }
    @Override
    public void onError(Throwable e) {
        mOnError.doAction(e);
    }
    @Override
    public void onComplete() {
        mOnComplete.doAction();
    }
}
