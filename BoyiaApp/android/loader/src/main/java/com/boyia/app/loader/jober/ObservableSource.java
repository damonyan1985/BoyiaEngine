package com.boyia.app.loader.jober;

/*
 * ObservableSource
 * Author yanbo.boyia
 * All Copyright reserved
 */
public interface ObservableSource<T> {
    void subscribe(Subscriber<? super T> observer);
}
