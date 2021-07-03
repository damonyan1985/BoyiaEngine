package com.boyia.app.loader.mue;

/*
 * ObservableOnSubscribe
 * Author yanbo.boyia
 * All Copyright reserved
 */
public interface ObservableOnSubscribe<T> {
    void subscribe(Subscriber<T> emitter) throws Exception;
}
