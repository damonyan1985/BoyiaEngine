package com.boyia.app.loader.mue;

/*
 * MueTaskCreate
 * Author yanbo.boyia
 * All Copyright reserved
 * 第一次创建MueTask的实例
 */
public class MueTaskCreate<T> extends MueTask<T> {
    private final MueTaskOnSubscribe<T> mSource;

    public MueTaskCreate(MueTaskOnSubscribe<T> source) {
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

        // 对应subscribe函数中对应的接口
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
