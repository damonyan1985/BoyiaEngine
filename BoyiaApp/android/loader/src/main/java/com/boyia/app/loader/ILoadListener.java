package com.boyia.app.loader;

/*
 * ILoadListener
 * Author yanbo.boyia
 * Date 2018-12-31
 * Copyright reserved
 * Descrption HTTP Resource Download Listener Interface
 */
public interface ILoadListener {
    // Begin to start load
    // 前期的准备工作，判断是否需要继续执行
    default boolean onLoadStart() {
        return true;
    }

    // Content-length return
    void onLoadDataSize(long size, Object msg);

    // Get redirect url
    void onLoadRedirectUrl(String redirectUrl);

    // Receive Data from loader
    void onLoadDataReceive(byte[] data, int len, Object msg);

    // Load data finished
    void onLoadFinished(Object msg);

    // Load data error
    void onLoadError(String error, Object msg);
}