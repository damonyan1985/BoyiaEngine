package com.boyia.app.loader.jober;

import com.boyia.app.loader.job.Scheduler;

public class ObservableObserveOn<T> extends Observable<T> {
    private final ObservableSource<T> mSource;
    private final Scheduler mScheduler;

    public ObservableObserveOn(ObservableSource<T> source, Scheduler scheduler) {
        mSource = source;
        mScheduler = scheduler;
    }

    // 1. 如果由observeOn创建的Observable执行此方法，则mSource为subscribeOn创建的Observable
    // 2. 如果由subscribeOn创建的Observable执行此方法，则mSource为ObservableCreate
    // 3. ObservableCreate执行subscribe，会执行Subscriber中的内容
    // 4. Subscriber执行包装了传入的Subscriber对象的EventEmitter
    // 5. EventEmitter相关方法执行会回调传入的Subscriber中对应的方法
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
