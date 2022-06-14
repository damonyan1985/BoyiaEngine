package com.boyia.app.shell.client;

import com.boyia.app.loader.ILoadListener;

public interface BoyiaSimpleLoaderListener extends ILoadListener {
    @Override
    default void onLoadDataSize(long size, Object msg) {
    }

    @Override
    default void onLoadRedirectUrl(String url) {
    }
}
