package com.boyia.app.loader.image;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.BoyiaLoader;
import com.boyia.app.loader.ILoadListener;
import java.io.ByteArrayOutputStream;

/*
 * BoyiaImageRunner
 * @Author yanbo.boyia
 * @Time 2018-8-31
 * @Copyright Reserved
 * @Descrption Image Resource Download
 *     Callback Implements ILoadListener
 */
public class BoyiaImageRunner implements ILoadListener {
    private static final String TAG = "BoyiaImageRunner";
    private long mDataSize;
    private boolean mNeedCached;
    private ByteArrayOutputStream mByteStream;
    private BoyiaImageHandler mHandler;

    protected BoyiaImageRunner(String url, boolean needCached) {
        mHandler = new BoyiaImageHandler(url);
        mNeedCached = needCached;
        mByteStream = new ByteArrayOutputStream();
    }

    public void startLoad() {
        mHandler.setLoader(new BoyiaLoader(this));
    }

    @Override
    public void onLoadStart() {
        BoyiaLog.d(TAG, "BoyiaImageRunner onLoadStart");
    }

    @Override
    public void onLoadFinished(Object msg) {
        // Maybe the picture is broken
        byte[] data = mByteStream.toByteArray();
        if (data.length < mDataSize) {
            BoyiaLog.d(TAG, "Image data less than we need, reload again");
            mHandler.retry();
            return;
        }

        mHandler.decode(data);

        if (mNeedCached) {
            mHandler.cacheDisk(data);
        }

        mHandler.release();
    }

    @Override
    public void onLoadError(String error, Object msg) {
        BoyiaLog.d(TAG, "BoyiaImageRunner onLoaderError: " + error);
        mHandler.retry();
    }

    @Override
    public void onLoadDataSize(long size) {
        BoyiaLog.d(TAG, "BoyiaImageRunner size: " + size);
        mDataSize = size;
    }

    @Override
    public void onLoadDataReceive(byte[] data, int length, Object msg) {
        mByteStream.write(data, 0, length);
    }

    @Override
    public void onLoadRedirectUrl(String redirectUrl) {
        mHandler.handleRedirect(redirectUrl);
    }

}
