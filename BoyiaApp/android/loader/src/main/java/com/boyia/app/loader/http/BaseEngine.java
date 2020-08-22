package com.boyia.app.loader.http;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;

/*
 * BaseEngine
 * Author yanbo.boyia
 * All Copyright reserved
 */
public abstract class BaseEngine {
    public static final String TAG = "BaseEngine";
    public static final String BOYIA_SCHEMA = "boyia://";
    public static final String BOYIA_ASSETS_SCHEMA = "boyia_assets://";

    public abstract void stop();

    public abstract Response getResponse(Request request);

    private InputStream boyiaAssetsStream(String url) throws IOException {
        if (url.startsWith(BOYIA_ASSETS_SCHEMA)) {
            int length = BOYIA_ASSETS_SCHEMA.length();
            String path = url.substring(length);
            return BaseApplication.getInstance().getAssets().open(path);
        }

        return null;
    }

    private InputStream boyiaAppStream(String url) throws FileNotFoundException {
        if (url.startsWith(BOYIA_SCHEMA)) {
            int length = BOYIA_SCHEMA.length();
            String path = url.substring(length);
            File fileDir = BaseApplication.getInstance().getFilesDir();
            File file = new File(fileDir, path);

            BoyiaLog.d(TAG, "path=" + path);
            BoyiaLog.d(TAG, "file path=" + file.getPath());
            return new FileInputStream(file);
        }

        return null;
    }

    protected Response getLocalResponse(String url) {
        if (BaseApplication.getInstance() == null) {
            return null;
        }

        Response data = new Response();
        try {
            InputStream stream = boyiaAssetsStream(url);
            if (stream == null) {
                stream = boyiaAppStream(url);
            }

            if (stream == null) {
                return null;
            }

            data.mInput = stream;
            data.mLength = data.mInput.available();
            data.mCode = HttpURLConnection.HTTP_OK;
        } catch (Exception e) {
            e.printStackTrace();
            data.mInput = null;
            data.mLength = 0;
            data.mCode = HttpURLConnection.HTTP_NOT_FOUND;
            data.mError = ErrorInfo.ERROR_RESOURCE_NOT_FOUNT;
        }

        return data;
    }

    protected static BaseEngine getHttpEngine(int type) {
        BaseEngine httpEngine;
        switch (type) {
            case HTTPFactory.URL_ENGINE:
                httpEngine = new URLEngine();
                break;
            case HTTPFactory.OK_ENGINE:
                httpEngine = new OkEngine();
                break;
            default:
                httpEngine = new OkEngine();
                break;
        }

        return httpEngine;
    }
}
