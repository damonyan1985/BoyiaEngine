package com.boyia.app.shell.client;

import com.boyia.app.loader.ILoadListener;

public interface BoyiaSimpleLoaderListener extends ILoadListener {
    @Override
    default void onLoadStart() {
    }

    @Override
    default void onLoadDataSize(long size) {
    }

    @Override
    default void onLoadRedirectUrl(String url) {
    }
}
