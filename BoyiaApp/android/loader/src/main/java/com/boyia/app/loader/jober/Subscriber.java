package com.boyia.app.loader.jober;

public interface Subscriber<T> {
    void onNext(T t);
    void onError(Throwable e);
    void onComplete();
}
