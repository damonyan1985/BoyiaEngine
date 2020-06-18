package com.boyia.app.loader.http;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
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

    public abstract void stop();

    public abstract Response getResponse(Request request);

    protected boolean isBoyiaPath(String url) {
        return url.startsWith(BOYIA_SCHEMA);
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
        if (isBoyiaPath(url) && BaseApplication.getInstance() != null) {
            Response data = new Response();
            try {
                data.mInput = boyiaAppStream(url);
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

        return null;
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
