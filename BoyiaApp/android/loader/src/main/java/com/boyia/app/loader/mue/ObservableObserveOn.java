package com.boyia.app.loader.mue;

import com.boyia.app.loader.job.IScheduler;

/*
 * ObservableObserveOn
 * Author yanbo.boyia
 * All Copyright reserved
 */
public class ObservableObserveOn<T> extends Observable<T> {
    private final ObservableSource<T> mSource;
    private final IScheduler mScheduler;

    public ObservableObserveOn(ObservableSource<T> source, IScheduler scheduler) {
        mSource = source;
        mScheduler = scheduler;
    }

    // 1. 如果由observeOn创建的Observable执行此方法，则mSource为subscribeOn创建的Observable
    // 2. 由subscribeOn创建的Observable为ObservableSubscribeOn类对象执行此方法
    // 3，ObservableSubscribeOn对象执行subscribe时会先执行schedule，然后调用ObservableCreate.subscribe
    // 4. ObservableCreate执行subscribe，传入ObservableObserveOn包装的Subscriber
    // 5. ObservableCreate.subscribe执行包装了传入的Subscriber对象的EventEmitter
    // 6. EventEmitter相关方法执行会回调传入的Subscriber中对应的方法
    // 7. 最终会执行ObservableObserveOn的mScheduler中
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
