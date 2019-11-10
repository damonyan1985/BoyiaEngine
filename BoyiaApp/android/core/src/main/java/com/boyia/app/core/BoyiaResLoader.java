package com.boyia.app.core;

import com.boyia.app.loader.ILoadListener;
import com.boyia.app.loader.BoyiaLoader;
import com.boyia.app.loader.http.HTTPFactory;
import com.boyia.app.loader.http.Response;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;

public class BoyiaResLoader implements ILoadListener {
	private BoyiaLoader mLoader = null;

    public BoyiaResLoader() {
    }
    
    public void beginRequest(int method) {
    	mLoader = new BoyiaLoader(this);
    	mLoader.setMethod(method);
    }
    
    public void putParam(String data) {
    	mLoader.putParam(data);
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
	public void onLoaderDataSize(long size) {
	}

	@Override
	public void onLoaderRedirectUrl(String redirectUrl) {
	}

	@Override
	public void onLoaderDataReceive(byte[] data, int length, Object msg) {
		ResInfo info = (ResInfo) msg;
		BoyiaUIView.nativeOnDataReceive(data, length, info.mCallback);
	}

	@Override
	public void onLoaderFinished(Object msg) {
    	ResInfo info = (ResInfo) msg;
    	BoyiaUIView.nativeOnDataFinished(info.mCallback);
	}

	@Override
	public void onLoaderError(String error, Object msg) {
		ResInfo info = (ResInfo) msg;
		info.mData = error;
		BoyiaUIView.nativeOnLoadError(info.mData, info.mCallback);
	}

	// Call By JNI
	public static String syncLoadResource(String url) {
		Response data = HTTPFactory.getResponse(url);
		InputStream in = data.getStream();
		ByteArrayOutputStream outStream = new ByteArrayOutputStream();

		byte[] buffer = new byte[1024];
		String result = "";

		try {
			int len;
			while ((len = in.read(buffer)) != -1) {
				outStream.write(buffer, 0, len);
			}

			result = new String(outStream.toByteArray(), HTTPFactory.HTTP_CHARSET_UTF8);
			outStream.close();
			in.close();
		} catch (Exception e) {
			e.printStackTrace();
		}

		return result;
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
