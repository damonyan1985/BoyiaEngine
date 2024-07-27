package com.boyia.app.loader.mue;

/**
 * 对执行的结果数据进行转换
 * 比如将json转换为实体对象
 * @param <T>
 * @param <R>
 */
public class MueTaskMap<T, R> extends MueTask<R> {
    private final IMueTask<T> mSource;
    private final MueFunction<T, R> mFunction;

    public MueTaskMap(IMueTask<T> source, MueFunction<T, R> function) {
        mSource = source;
        mFunction = function;
    }
    @Override
    void subscribeActual(Subscriber<? super R> observer) {
        mSource.subscribe(new MapObserver<>(observer, mFunction));
    }

    static class MapObserver<T, R> implements Subscriber<T> {
        final Subscriber<? super R> mObserver;
        final MueFunction<T, R> mFunction;

        public MapObserver(Subscriber<? super R> observer,  MueFunction<T, R> function) {
            mObserver = observer;
            mFunction = function;
        }
        @Override
        public void onNext(T t) {
            // apply在onnext的时候处理
            R result = mFunction.apply(t);
            mObserver.onNext(result);
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
