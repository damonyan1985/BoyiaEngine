package com.boyia.app.core;

import java.io.UnsupportedEncodingException;

import com.boyia.app.http.HttpUtil;
import com.boyia.app.loader.ILoaderCallback;
import com.boyia.app.loader.BoyiaLoader;

public class ResourceLoader implements ILoaderCallback {
	private BoyiaLoader mLoader = null;

    public ResourceLoader() {
    }
    
    public void beginRequest(int method) {
    	mLoader = new BoyiaLoader(this);
    	mLoader.setMethod(method);
    }
    
    public void putParam(String key, String value) {
    	mLoader.putParam(key, value);
    }
    
    public void putHeader(String key, String value) {
    	mLoader.putHeader(key, value);
    }
    
    public void endRequest(String url, long callback) {
    	endRequest(url, true, callback);
    }
    
    public void endRequest(String url, boolean isWait, long callback) {
    	mLoader.load(url, isWait, new ResInfo(url, callback));
    }

	@Override
	public void onLoaderStart() {
	}

	@Override
	public void onLoaderDataLen(int size) {
	}

	@Override
	public void onLoaderRedirectUrl(String redirectUrl) {
	}

	@Override
	public void onLoaderDataReceive(byte[] data, int length) {
	}

	@Override
	public void onLoaderFinished(byte[] data, Object msg) {
		try {	
			ResInfo info = (ResInfo) msg;
			info.mData = new String(data, HttpUtil.HTTP_CHARSET_UTF8);
			BoyiaUIView.nativeOnDataFinished(info.mData, info.mCallback);
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void onLoaderError(String error, Object msg) {
		ResInfo info = (ResInfo) msg;
		info.mData = error;
		BoyiaUIView.nativeOnLoadError(info.mData, info.mCallback);
	}
	
	public static class ResInfo {
		public String mUrl;
		public long mCallback;
		public String mData;
		
		public ResInfo(String url, long callback) {
			mUrl = url;
			mCallback = callback;
			mData = null;
		}
	}
}
