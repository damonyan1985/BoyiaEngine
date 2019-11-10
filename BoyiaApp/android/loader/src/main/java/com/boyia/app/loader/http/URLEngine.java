package com.boyia.app.loader.http;

import com.boyia.app.common.utils.BoyiaLog;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.SocketTimeoutException;
import java.net.URL;
import java.util.Date;
import java.util.Map.Entry;


public class URLEngine extends BaseEngine {
    private static final String TAG = URLEngine.class.getSimpleName();
    private HttpURLConnection mConnection = null;

    @Override
    public void stop() {
        if (mConnection != null) {
            mConnection.disconnect();
        }
    }

    @Override
    public Response getResponse(Request request) {
        Response data = getLocalResponse(request.mUrl);
        if (data == null) {
            BoyiaLog.d(TAG, "The http url: " + request.mUrl);
            data = new Response();
            URL netUrl;
            try {
                netUrl = new URL(request.mUrl);
            } catch (MalformedURLException e1) {
                e1.printStackTrace();
                BoyiaLog.d(TAG, "The http error: " + e1.toString());
                data.mError = ErrorInfo.ERROR_URL_NOT_VALID;
                data.mCode = HttpURLConnection.HTTP_BAD_REQUEST;
                return data;
            }

            try {
                executeHttpSetting(netUrl, request);
                fetchStream(data);
            } catch (SocketTimeoutException ste) {
                ste.printStackTrace();
                BoyiaLog.d(TAG, "The http error: " + ste.toString());
                data.mError = ErrorInfo.ERROR_TIME_OUT;
            } catch (IOException ioe) {
                ioe.printStackTrace();
                BoyiaLog.d(TAG, "The http error: " + ioe.toString());
                data.mError = ErrorInfo.ERROR_IO_EXCEPTION;
            } catch (Exception e) {
                e.printStackTrace();
                BoyiaLog.d(TAG, "The http error: " + e.toString());
                data.mError = e.toString();
            }
        }

        return data;
    }

    private void executeHttpSetting(URL netUrl, Request request) throws IOException {
        mConnection = (HttpURLConnection) netUrl.openConnection();
        // Not allowed auto skip to another link
        mConnection.setInstanceFollowRedirects(false);
        mConnection.setReadTimeout(HTTPFactory.HTTP_TIME_OUT);
        mConnection.setConnectTimeout(HTTPFactory.HTTP_TIME_OUT);
        if (request.mHeaders != null && request.mHeaders.size() > 0) {
            for (Entry<String, String> entry : request.mHeaders
                    .entrySet()) {
                mConnection.setRequestProperty(entry.getKey(),
                        entry.getValue());
            }
        }

        switch (request.mMethod) {
            case HTTPFactory.HTTP_GET_METHOD:
                mConnection.setRequestMethod(HTTPFactory.HTTP_GET_METHOD_STR);
                break;
            case HTTPFactory.HTTP_POST_METHOD:
                postParams(mConnection, request);
                break;
        }
    }

    private void fetchStream(Response response) throws IOException {
        if (mConnection != null) {
            response.mCode = mConnection.getResponseCode();
            if (response.mCode == HttpURLConnection.HTTP_OK
                    || response.mCode == HttpURLConnection.HTTP_PARTIAL) {
                response.mLength = mConnection.getContentLength();
                response.mInput = mConnection.getInputStream();
                response.mHeaders = mConnection.getHeaderFields();
            } else if (response.mCode == HttpURLConnection.HTTP_MOVED_TEMP) {
                response.mLength = mConnection.getContentLength();
                response.mHeaders = mConnection.getHeaderFields();
                BoyiaLog.d(TAG, "The http move to other address");
            } else {
                BoyiaLog.d(TAG, "The http status code: " + response.mCode);
            }
        }
    }

    private void postParams(HttpURLConnection conn, Request request) {
        byte[] data = request.mPostData.getBytes();
        if (request.mMethod == HTTPFactory.HTTP_POST_METHOD) {
            conn.setDoOutput(true);
            conn.setDoInput(true);
            conn.setUseCaches(false);
        }

        try {
            conn.connect();
            OutputStream outputStream = conn.getOutputStream();
            if (outputStream != null) {
                DataOutputStream dos = new DataOutputStream(outputStream);
                dos.write(data, 0, data.length);
                dos.flush();
                dos.close();
            }

            BufferedReader br = new BufferedReader(new InputStreamReader(
                    conn.getInputStream(), "UTF-8"));
            // Waiting for data call back
            Date date = new Date();
            long startWaitTime = date.getTime();
            while (!br.ready()) {
                long nowWaitTime = new Date().getTime();
                if (nowWaitTime - startWaitTime > HTTPFactory.HTTP_TIME_OUT) {
                    break;
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
