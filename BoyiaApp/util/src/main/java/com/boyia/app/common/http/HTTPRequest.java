package com.boyia.app.common.http;

import java.util.HashMap;
import java.util.Map;

/*
 * HttpRequestData
 * @Author Yan bo
 * @Time 2014-5-4
 * @Copyright Reserved
 * @Descrption HTTP Resource Request Data
 *  Interface
 */

public class HTTPRequest {
	public Map<String, String> mHeaders;
	public Map<String, String> mPostParams;
	public int mMethod;
	public String mUrl;
	
	public HTTPRequest() {
		this(null, HTTPUtil.HTTP_GET_METHOD, null, null);
	}

	public HTTPRequest(String url, int method,
                       Map<String, String> headers, Map<String, String> params) {
		mMethod = method;
		mHeaders = headers;
		mPostParams = params;
		mUrl = url;
	}

	public void putHeader(String key, String value) {
		if (mHeaders == null) {
			mHeaders = new HashMap<String, String>();
		}

		mHeaders.put(key, value);
	}

	public void putParam(String key, String value) {
		if (mPostParams == null) {
			mPostParams = new HashMap<String, String>();
		}

		mPostParams.put(key, value);
	}
	
	public void resetRequestData() {
		mHeaders = null;
//		if (mHeaders != null) {
//			mHeaders.clear();
//		}
		mPostParams = null;
		mMethod = HTTPUtil.HTTP_GET_METHOD;
		mUrl = null;
	}
}