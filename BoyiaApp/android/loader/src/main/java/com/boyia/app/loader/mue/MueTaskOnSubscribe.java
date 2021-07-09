package com.boyia.app.loader.mue;

/*
 * MueTaskOnSubscribe
 * Author yanbo.boyia
 * All Copyright reserved
 * 发起订阅，最终需要处理的订阅事件是emitter
 * emitter中会封装并执行最终的观察者
 */
public interface MueTaskOnSubscribe<T> {
    void subscribe(Subscriber<T> emitter) throws Exception;
}
