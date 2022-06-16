package com.boyia.app.loader;

import com.boyia.app.loader.http.HTTPFactory;
import com.boyia.app.loader.http.Request;
import com.boyia.app.loader.http.BoyiaLoadJob;
import com.boyia.app.loader.job.JobScheduler;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;

/*
 * BoyiaLoader
 * Author yanbo.boyia
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
        mRequest.mMsg = msg;
        mLoadJob = new BoyiaLoadJob(mRequest, this);
        if (isWait) {
            BoyiaWorker.getWorker().sendJob(mLoadJob);
        } else {
            JobScheduler.jobScheduler().sendJob(mLoadJob);
        }
    }

    public void load(String url, boolean isWait) {
        load(url, isWait, null);
    }

    public void upload(String url, boolean isWait, Object msg) {
        mRequest.mUrl = url;
        mRequest.mMsg = msg;
        mRequest.mMethod = HTTPFactory.HTTP_POST_UPLOAD_METHOD;

        mLoadJob = new BoyiaLoadJob(mRequest, this);
        if (isWait) {
            BoyiaWorker.getWorker().sendJob(mLoadJob);
        } else {
            JobScheduler.jobScheduler().sendJob(mLoadJob);
        }
//        IJob job = () -> Uploader.upload(mRequest, mListener, msg);
//
//        if (isWait) {
//            BoyiaWorker.getWorker().sendJob(job);
//        } else {
//            JobScheduler.jobScheduler().sendJob(job);
//        }
    }

    @Override
    public void onDataReceived(byte[] data, int length) {
        if (mListener != null) {
            mListener.onLoadDataReceive(data, length, mRequest.mMsg);
        }
    }

    @Override
    public void onReceiveFinished() {
        BoyiaLog.i(TAG, "onReceiveFinished");
        if (mListener != null) {
            mListener.onLoadFinished(mRequest.mMsg);
        }
    }

    @Override
    public void onDataSize(long size) {
        if (mListener != null) {
            mListener.onLoadDataSize(size, mRequest.mMsg);
        }
    }

    @Override
    public void onHttpError(String info) {
        if (mListener != null) {
            mListener.onLoadError(info, mRequest.mMsg);
        }
    }

    @Override
    public void onUploadProgress(long current, long total) {
        if (mListener != null) {
            mListener.onUploadProgress(current, total, mRequest.mMsg);
        }
    }

    @Override
    public boolean onLoadStart() {
        if (mListener != null) {
            return mListener.onLoadStart();
        }

        // 默认继续执行
        return true;
    }

    @Override
    public void onRedirectUrl(String redirectUrl) {
        if (mListener != null) {
            mListener.onLoadRedirectUrl(redirectUrl);
        }
    }
}
