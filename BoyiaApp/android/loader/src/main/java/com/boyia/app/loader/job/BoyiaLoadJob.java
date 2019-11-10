package com.boyia.app.loader.job;

import java.io.IOException;

import com.boyia.app.loader.http.BaseEngine;
import com.boyia.app.loader.http.ErrorInfo;
import com.boyia.app.loader.http.Request;
import com.boyia.app.loader.http.Response;
import com.boyia.app.loader.http.HTTPFactory;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;

/*
 * BoyiaLoadJob
 * Author Boyia
 * Date 2018-8-31
 * Copyright Reserved
 */
public class BoyiaLoadJob implements IJob {
    private static final String TAG = BoyiaLoadJob.class.getSimpleName();
    public static final int REQUEST_BUFFER_SIZE = 1024;

    private LoadJobCallback mCallback;
    private Request mRequest;
    private BaseEngine mHttpEngine;
    private boolean mHasStop = false;
    private Object mMessage;

    public interface LoadJobCallback {
        void onLoadStart();

        void onRedirectUrl(String redirectUrl);

        void onDataReceived(byte[] data, int length, Object msg);

        void onReceiveFinished(Object msg);

        void onDataSize(long size);

        void onHttpError(String info, Object msg);
    }

    public BoyiaLoadJob(Request request,
                        LoadJobCallback callback) {
        this(request, callback, null);
    }

    public BoyiaLoadJob(Request request,
                        LoadJobCallback callback,
                        Object message) {
        mCallback = callback;
        mRequest = request;
        mMessage = message;
        mHttpEngine = HTTPFactory.createHttpEngine();
    }

    public void setCallback(LoadJobCallback callback) {
        mCallback = callback;
    }

    public void setRequest(Request request) {
        mRequest = request;
    }

    public void stop() {
        mHasStop = true;
        mHttpEngine.stop();
    }

    @Override
    public void exec() {
        BoyiaLog.i(TAG, "BoyiaLoadJob execute");
        if (null != mCallback) {
            mCallback.onLoadStart();
        }

        Response data = mHttpEngine.getResponse(mRequest);

        if (null == data) {
            BoyiaLog.d(TAG, "BoyiaLoadJob request url: " + mRequest.mUrl);
            executeError(ErrorInfo.ERROR_DEFAULT_LOAD_ERROR);
            return;
        }

        if (!executeRedirect(data)) {
            return;
        }

        if (null == data.getStream()) {
            executeError(data.getError());
            return;
        }

        if (!executeReceive(data)) {
            return;
        }

        if (null != mCallback) {
            mCallback.onReceiveFinished(mMessage);
        }
    }

    private boolean executeRedirect(Response data) {
        if (null != mCallback) {
            mCallback.onDataSize(data.getLength());
            String redirectUrl = data.getRedirectUrl();
            if (!BoyiaUtils.isTextEmpty(redirectUrl)) {
                BoyiaLog.d(TAG, "BoyiaLoadJob redirectUrl: " + redirectUrl);
                mCallback.onRedirectUrl(redirectUrl);
                return false;
            }
        }

        return true;
    }

    private boolean executeReceive(Response data) {
        byte[] buffer = new byte[REQUEST_BUFFER_SIZE];
        int length;
        try {
            while ((length = data.read(buffer)) != -1 && !mHasStop) {
                if (null != mCallback) {
                    mCallback.onDataReceived(buffer, length, mMessage);
                }
            }

            BoyiaLog.d(TAG, "BoyiaLoadJob receive data success url: "
                    + mRequest.mUrl);
            data.close();
        } catch (IOException e) {
            e.printStackTrace();
            executeError(e.toString());
            return false;
        }

        return true;
    }

    private void executeError(String error) {
        if (null != mCallback) {
            if (mHasStop) {
                mCallback.onReceiveFinished(mMessage);
            } else {
                mCallback.onHttpError(error, mMessage);
            }
        }
    }
}
