package com.boyia.app.loader.jober;

import com.boyia.app.loader.job.Scheduler;

public class ObservableCreate<T> extends Observable<T> {
    private final ObservableOnSubscribe<T> mSource;
    public ObservableCreate(ObservableOnSubscribe<T> source) {
        mSource = source;
    }

    private void subscribeImpl(Subscriber<? super T> observer) {
        CreateEmitter<T> emitter = new CreateEmitter<>(observer, mObserveOnScheduler);
        try {
            // 执行任务的地方
            mSource.subscribe(emitter);
        } catch (Exception e) {
            emitter.onError(e);
        }
    }

    @Override
    void subscribeActual(Subscriber<? super T> observer) {
        if (mSubscribeOnScheduler != null) {
            mSubscribeOnScheduler.sendJob(() -> {
                subscribeImpl(observer);
            });
        } else {
            subscribeImpl(observer);
        }
    }

    static class CreateEmitter<T> implements Subscriber<T> {
        private final Subscriber<? super T> mObserver;
        private final Scheduler mScheduler;

        CreateEmitter(Subscriber<? super T> observer, Scheduler scheduler) {
            mObserver = observer;
            mScheduler = scheduler;
        }

        @Override
        public void onNext(T t) {
            if (mScheduler != null) {
                mScheduler.sendJob(() -> {
                    mObserver.onNext(t);
                });
            } else {
                mObserver.onNext(t);;
            }

        }
        @Override
        public void onError(Throwable e) {
            if (mScheduler != null) {
                mScheduler.sendJob(() -> {
                    mObserver.onError(e);
                });
            } else {
                mObserver.onError(e);
            }
        }
        @Override
        public void onComplete() {
            if (mScheduler != null) {
                mScheduler.sendJob(() -> {
                    mObserver.onComplete();
                });
            } else {
                mObserver.onComplete();
            }
        }
    }
}
