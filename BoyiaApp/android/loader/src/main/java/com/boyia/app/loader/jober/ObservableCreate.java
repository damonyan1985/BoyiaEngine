package com.boyia.app.loader.jober;

public class ObservableCreate<T> extends Observable<T> {
    private final ObservableOnSubscribe<T> mSource;

    public ObservableCreate(ObservableOnSubscribe<T> source) {
        mSource = source;
    }

    private void subscribeImpl(Subscriber<? super T> observer) {
        EmitterImpl<T> emitter = new EmitterImpl<>(observer);
        try {
            mSource.subscribe(emitter);
        } catch (Exception e) {
            emitter.onError(e);
        }
    }

    @Override
    void subscribeActual(Subscriber<? super T> observer) {
        subscribeImpl(observer);
    }

    static class EmitterImpl<T> implements Subscriber<T> {
        // 该mObserver为subscribe传入的对象
        private final Subscriber<? super T> mObserver;

        EmitterImpl(Subscriber<? super T> observer) {
            mObserver = observer;
        }

        @Override
        public void onNext(T t) {
            mObserver.onNext(t);
        }

        @Override
        public void onError(Throwable e) {
            mObserver.onError(e);
        }

        @Override
        public void onComplete() {
            mObserver.onComplete();
        }
    }
}
