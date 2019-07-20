package com.boyia.app.common.http;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;

import com.boyia.app.common.BoyiaApplication;
import com.boyia.app.common.utils.BoyiaLog;

import android.content.res.AssetManager;

public abstract class BaseHTTPEngine {

	public abstract void stop();

    public abstract HTTPResponse getStream(HTTPRequest request);
    
    protected boolean isLocalPath(String url) {
    	return url.startsWith(BoyiaApplication.BOYIA_DIR)
    			|| url.startsWith(BoyiaApplication.BOYIA_SDK_DIR);
    }
    
    private InputStream localSdkStream(String url) throws IOException {
    	if (url.startsWith(BoyiaApplication.BOYIA_SDK_DIR)) {
    		AssetManager asset = BoyiaApplication.getCurrenContext().getAssets();
    		int length = BoyiaApplication.BOYIA_SDK_DIR.length();
			String path = url.substring(length, url.length());
			return asset.open(path);
    	}
    	
    	return null;
    }
    
    private InputStream localAppStream(String url) throws FileNotFoundException {
    	if (url.startsWith(BoyiaApplication.BOYIA_DIR)) {
    		int length = BoyiaApplication.BOYIA_DIR.length();
			String path = url.substring(length, url.length());
			File fileDir = BoyiaApplication.getCurrenContext().getFilesDir();
			File file = new File(fileDir, path);
			
			BoyiaLog.d("HttpEngine", "path=" + path);
			BoyiaLog.d("HttpEngine", "file path=" + file.getPath());
			return new FileInputStream(file);
    	}
    	
    	return null;
    }
    
	protected HTTPResponse getLocalData(String url) {
		if (isLocalPath(url) && BoyiaApplication.getCurrenContext() != null) {			
			HTTPResponse data = new HTTPResponse();
			try {
				InputStream input = localSdkStream(url);
				if (input == null) {
					input = localAppStream(url);
				}
				
				data.mInput = input;
				data.mLength = data.mInput.available();
				data.mCode = HttpURLConnection.HTTP_OK;
			} catch (Exception e) {
				e.printStackTrace();
				data.mInput = null;
				data.mLength = 0;
				data.mCode = HttpURLConnection.HTTP_NOT_FOUND;
				data.mError = HTTPErrorInfo.HTTP_RESOURCE_NOT_FOUNT;
			}

			return data;
		}

		return null;
	}
	
	protected static BaseHTTPEngine getHttpEngine(int type) {
		BaseHTTPEngine httpEngine;
		switch (type) {
		case HTTPUtil.URL_ENGINE:
			httpEngine = new URLHTTPEngine();
			break;
		case HTTPUtil.OK_ENGINE:
			httpEngine = new OKHTTPEngine();
			break;
		default:
			httpEngine = new OKHTTPEngine();
			break;
		}
		
		return httpEngine;
	}
}
