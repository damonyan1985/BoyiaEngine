package com.boyia.app.common.http;

import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.ProtocolException;
import java.net.SocketTimeoutException;
import java.net.URL;
import java.util.Date;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import com.boyia.app.common.utils.BoyiaLog;


public class URLHTTPEngine extends BaseHTTPEngine {
    private HttpURLConnection mConnection = null;

    @Override
    public void stop() {
        if (mConnection != null) {
            mConnection.disconnect();
        }
    }

    @Override
    public HTTPResponse getStream(HTTPRequest request) {
        HTTPResponse data = getLocalData(request.mUrl);
        if (data == null) {
            BoyiaLog.d("HttpEngine", "loader url=" + request.mUrl);
            data = new HTTPResponse();
            URL netUrl;
            try {
                netUrl = new URL(request.mUrl);
            } catch (MalformedURLException e1) {
                e1.printStackTrace();
                BoyiaLog.d("HttpEngine", "loader error=" + e1.toString());
                data.mError = HTTPErrorInfo.HTTP_URL_NOT_FOMAT;
                data.mCode = HttpURLConnection.HTTP_BAD_REQUEST;
                return data;
            }

            try {
                executeHttpSetting(netUrl, request);
                fetchStream(data);
            } catch (SocketTimeoutException e) {
                e.printStackTrace();
                BoyiaLog.d("HttpEngine", "loader error=" + e.toString());
                data.mError = HTTPErrorInfo.HTTP_TIME_OUT;
            } catch (IOException e1) {
                e1.printStackTrace();
                BoyiaLog.d("HttpEngine", "loader error=" + e1.toString());
                data.mError = HTTPErrorInfo.HTTP_IO_EXCEPTION;
            } catch (Exception e) {
                e.printStackTrace();
                BoyiaLog.d("HttpEngine", "loader error=" + e.toString());
                data.mError = e.toString();
            }
        }

        return data;
    }

    private void executeHttpSetting(URL netUrl, HTTPRequest request) throws IOException {
        mConnection = (HttpURLConnection) netUrl.openConnection();
        // 不允许自动302跳转
        mConnection.setInstanceFollowRedirects(false);
        mConnection.setReadTimeout(HTTPUtil.HTTP_TIME_OUT);
        mConnection.setConnectTimeout(HTTPUtil.HTTP_TIME_OUT);
        if (request.mHeaders != null && request.mHeaders.size() > 0) {
            for (Entry<String, String> entry : request.mHeaders
                    .entrySet()) {
                mConnection.setRequestProperty(entry.getKey(),
                        entry.getValue());
            }
        }

        switch (request.mMethod) {
            case HTTPUtil.HTTP_GET_METHOD:
                mConnection.setRequestMethod(HTTPUtil.HTTP_GET_METHOD_STR);
                break;
            case HTTPUtil.HTTP_POST_METHOD:
                postParams(mConnection, request.mPostParams);
                break;
        }
    }

    private void fetchStream(HTTPResponse response) throws IOException {
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
                BoyiaLog.d("HttpEngine", "loader http move");
            } else {
                BoyiaLog.d("HttpEngine", "loader http code=" + response.mCode);
            }
        }
    }

    private void postParams(HttpURLConnection conn,
                            Map<String, String> requestParamsMap) {
        StringBuilder params = new StringBuilder();
        Iterator<Entry<String, String>> it = requestParamsMap.entrySet().iterator();
        while (it.hasNext()) {
            Entry<String, String> element = it.next();
            params.append(element.getKey());
            params.append("=");
            params.append(element.getValue());
            params.append("&");
        }

        if (params.length() > 0) {
            params.deleteCharAt(params.length() - 1);
        }

        byte[] data = params.toString().getBytes();

        conn.setDoOutput(true);
        conn.setDoInput(true);
        conn.setUseCaches(false);

        conn.setRequestProperty(HTTPUtil.HTTP_HEADER_CONTENT_TYPE,
                HTTPUtil.HTTP_HEADER_VALUE_FORM);
        conn.setRequestProperty(HTTPUtil.HTTP_HEADER_CONNECT, HTTPUtil.HTTP_HEADER_VALUE_KEEP);// 维持长连接
        conn.setRequestProperty(HTTPUtil.HTTP_HEADER_CHARSET, HTTPUtil.HTTP_CHARSET_UTF8);
        conn.setRequestProperty(HTTPUtil.HTTP_HEADER_CONTENT_LENGTH, String.valueOf(data.length));

        try {
            conn.setRequestMethod(HTTPUtil.HTTP_POST_METHOD_STR);
        } catch (ProtocolException e) {
            e.printStackTrace();
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
            // 一直等待
            Date date = new Date();
            long startWaitTime = date.getTime();
            while (!br.ready()) {
                long nowWaitTime = new Date().getTime();
                if (nowWaitTime - startWaitTime > HTTPUtil.HTTP_TIME_OUT) {
                    break;
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }

    }
}
