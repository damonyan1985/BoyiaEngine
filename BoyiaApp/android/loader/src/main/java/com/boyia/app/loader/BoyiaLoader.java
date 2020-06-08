package com.boyia.app.loader;

import com.boyia.app.loader.http.Request;
import com.boyia.app.loader.job.BoyiaLoadJob;
import com.boyia.app.loader.job.JobScheduler;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;

/*
 * BoyiaLoader
 * Author Yanbo
 * Time 2018-12-31
 * Copyright(c) Reserved
 */
public class BoyiaLoader implements BoyiaLoadJob.LoadJobCallback {
    private static final String TAG = "BoyiaLoader";
    protected ILoadListener mListener;
    protected BoyiaLoadJob mLoadJob;
    protected Request mRequest;

    public BoyiaLoader(ILoadListener listener) {
        mRequest = new Request();
        mListener = listener;
    }

    public void putRequestHeader(String key, String value) {
        mRequest.putHeader(key, value);
    }

    public void setPostParam(String data) {
        mRequest.putParam(data);
    }

    public void setRequestMethod(int method) {
        mRequest.mMethod = method;
    }

    public void reset() {
        reset(null);
    }

    public void reset(ILoadListener listener) {
        mRequest.resetRequestData();
        mLoadJob = null;
        mListener = listener;
    }

    public void setCallback(ILoadListener listener) {
        mListener = listener;
        mRequest.resetRequestData();
    }

    public void stop() {
        if (mLoadJob != null) {
            mLoadJob.stop();
        }
    }

    public void load(String url) {
        load(url, false);
    }

    public void load(String url, boolean isWait, Object msg) {
        BoyiaLog.d(TAG, "load---url:" + url);
        if (BoyiaUtils.isTextEmpty(url)) {
            return;
        }

        mRequest.mUrl = url;
        mLoadJob = new BoyiaLoadJob(mRequest, this, msg);
        if (isWait) {
            BoyiaWorker.getWorker().sendJob(mLoadJob);
        } else {
            JobScheduler.getInstance().sendJob(mLoadJob);
        }
    }

    public void load(String url, boolean isWait) {
        load(url, isWait, null);
    }

    @Override
    public void onDataReceived(byte[] data, int length, Object msg) {
        if (mListener != null) {
            mListener.onLoadDataReceive(data, length, msg);
        }
    }

    @Override
    public void onReceiveFinished(Object msg) {
        BoyiaLog.i(TAG, "onReceiveFinished");
        if (mListener != null) {
            mListener.onLoadFinished(msg);
        }
    }

    @Override
    public void onDataSize(long size) {
        if (mListener != null) {
            mListener.onLoadDataSize(size);
        }
    }

    @Override
    public void onHttpError(String info, Object msg) {
        if (mListener != null) {
            mListener.onLoadError(info, msg);
        }
    }

    @Override
    public void onLoadStart() {
        if (mListener != null) {
            mListener.onLoadStart();
        }
    }

    @Override
    public void onRedirectUrl(String redirectUrl) {
        if (mListener != null) {
            mListener.onLoadRedirectUrl(redirectUrl);
        }
    }
}
