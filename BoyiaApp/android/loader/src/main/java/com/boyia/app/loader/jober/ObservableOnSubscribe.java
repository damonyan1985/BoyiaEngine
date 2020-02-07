package com.boyia.app.loader.jober;

public interface ObservableOnSubscribe<T> {
    void subscribe(Subscriber<T> emitter) throws Exception;
}
