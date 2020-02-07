package com.boyia.app.loader.jober;

public interface ObservableSource<T> {
    void subscribe(Subscriber<? super T> observer);
}
