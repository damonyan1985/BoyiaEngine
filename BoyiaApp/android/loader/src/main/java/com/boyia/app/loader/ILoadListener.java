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
    void onLoadStart();

    // Content-length return
    void onLoadDataSize(long size);

    // Get redirect url
    void onLoadRedirectUrl(String redirectUrl);

    // Receive Data from loader
    void onLoadDataReceive(byte[] data, int len, Object msg);

    // Load data finished
    void onLoadFinished(Object msg);

    // Load data error
    void onLoadError(String error, Object msg);
}