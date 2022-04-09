package com.boyia.app.shell.client;

import com.boyia.app.loader.ILoadListener;

public abstract class BoyiaSimpleLoaderListener implements ILoadListener {
    @Override
    public void onLoadStart() {
    }

    @Override
    public void onLoadDataSize(long size) {
    }

    @Override
    public void onLoadRedirectUrl(String url) {
    }
}
