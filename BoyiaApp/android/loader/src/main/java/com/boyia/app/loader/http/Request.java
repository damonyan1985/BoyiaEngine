package com.boyia.app.loader.http;

import java.util.HashMap;
import java.util.Map;

/*
 * Request
 * @Author Boyia
 * @Date 2014-5-4
 * @Copyright Reserved
 */

public class Request {
    public Map<String, String> mHeaders;
    public String mPostData;
    public int mMethod;
    public String mUrl;

    public Request() {
        this(null, HTTPFactory.HTTP_GET_METHOD, null, null);
    }

    public Request(String url, int method,
                   Map<String, String> headers, String params) {
        mMethod = method;
        mHeaders = headers;
        mPostData = params;
        mUrl = url;
    }

    public void putHeader(String key, String value) {
        if (mHeaders == null) {
            mHeaders = new HashMap<>();
        }

        mHeaders.put(key, value);
    }

    public void putParam(String data) {
        mPostData = data;
    }

    public void resetRequestData() {
        mHeaders = null;
        mPostData = null;
        mMethod = HTTPFactory.HTTP_GET_METHOD;
        mUrl = null;
    }
}