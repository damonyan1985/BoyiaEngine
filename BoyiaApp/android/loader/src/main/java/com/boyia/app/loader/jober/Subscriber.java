package com.boyia.app.loader.jober;

/*
 * Subscriber
 * Author yanbo.boyia
 * All Copyright reserved
 */
public interface Subscriber<T> {
    void onNext(T t);
    void onError(Throwable e);
    void onComplete();
}
