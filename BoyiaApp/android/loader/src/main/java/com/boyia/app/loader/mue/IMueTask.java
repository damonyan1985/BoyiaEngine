package com.boyia.app.loader.mue;

/*
 * IMueTask
 * Author yanbo.boyia
 * All Copyright reserved
 */
public interface IMueTask<T> {
    void subscribe(Subscriber<? super T> observer);
}
