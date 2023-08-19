package com.boyia.app.loader.http;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.ILoadListener;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import okhttp3.Call;
import okhttp3.MediaType;
import okhttp3.MultipartBody;
import okhttp3.OkHttpClient;
import okhttp3.RequestBody;
import okio.Buffer;
import okio.BufferedSink;
import okio.Okio;
import okio.Source;

/*
 * OkEngine
 * Author yanbo.boyia
 * All Copyright reserved
 */
public class OkEngine extends BaseEngine {
    private Call mCall;

    @Override
    public void stop() {
        if (mCall != null) {
            mCall.cancel();
        }
    }

    private Response handleResponse(OkHttpClient client, okhttp3.Request.Builder builder) {
        try {
            Response response = new Response();
            mCall = client.newCall(builder.build());
            okhttp3.Response res = mCall.execute();
            response.mInput = res.body().byteStream();
            response.mCode = res.code();
            response.mLength = res.body().contentLength();

            Set<String> names = res.headers().names();
            Map<String, List<String>> resultHeaders = new HashMap<>();
            if (names != null && names.size() > 0) {
                for (String name : names) {
                    if (resultHeaders.containsKey(name)) {
                        resultHeaders.get(name).add(res.headers().get(name));
                    } else {
                        List<String> values = new ArrayList<>();
                        values.add(res.headers().get(name));
                        resultHeaders.put(name, values);
                    }
                }
            }

            response.mHeaders = resultHeaders;
            return response;
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        return null;
    }

    @Override
    public Response getResponse(Request request) {
        Response response = getLocalResponse(request.mUrl);
        if (response != null) {
            return response;
        }

        SSLHelper.SSLInfo info = SSLHelper.getSSLInfo();
        okhttp3.Request.Builder builder = setHeader(request);
        OkHttpClient client = new OkHttpClient().newBuilder()
                .connectTimeout(HTTPFactory.HTTP_TIME_OUT, TimeUnit.SECONDS)
                .readTimeout(HTTPFactory.HTTP_TIME_OUT, TimeUnit.SECONDS)
                .sslSocketFactory(info.mFactory, info.mTrustManagers[0])
                .hostnameVerifier((hostname, session) -> true)
                .build();

        switch (request.mMethod) {
            case HTTPFactory.HTTP_GET_METHOD:
                builder.get();
                break;
            case HTTPFactory.HTTP_POST_METHOD: {
                //final MediaType type = MediaType.parse("application/json; charset=utf-8");
                // Post json数据
                RequestBody body = RequestBody.create(null, request.mPostData);
                builder.post(body);
            }
                break;
            case HTTPFactory.HTTP_POST_UPLOAD_METHOD: {
                File file = new File(request.mPostData);
                MultipartBody.Builder postBodyBuilder = new MultipartBody.Builder().setType(MultipartBody.FORM);
                postBodyBuilder.addFormDataPart(
                        HTTPFactory.UPLOADER_TYPE_FILE,
                        file.getName(),
                        createUpload(request, MultipartBody.FORM, file));
                builder.post(postBodyBuilder.build());
            }
                break;
        }

        return handleResponse(client, builder);
    }

    private okhttp3.Request.Builder setHeader(Request request) {
        okhttp3.Request.Builder builder = new okhttp3.Request.Builder();
        builder.url(request.mUrl);
        if (request.mHeaders != null && request.mHeaders.size() > 0) {
            for (Map.Entry<String, String> entry : request.mHeaders
                    .entrySet()) {
                builder.addHeader(entry.getKey(), entry.getValue());
            }
        }

        return builder;
    }

    private static RequestBody createUpload(
            final Request request,
            final MediaType type,
            final File file) {
        return new RequestBody() {
            @Override
            public MediaType contentType() {
                return type;
            }

            public long contentLength() throws IOException {
                return file.length();
            }

            @Override
            public void writeTo(BufferedSink sink) throws IOException {
                Source source;
                try {
                    source = Okio.source(file);
                    Buffer buffer = new Buffer();

                    long total = contentLength();
                    long current = 0;
                    for (long readSize; (readSize = source.read(buffer, HTTPFactory.UPLOADER_WRITE_SIZE)) != -1;) {
                        sink.write(buffer, readSize);
                        if (request.mListener != null) {
                            current += readSize;
                            request.mListener.onUploadProgress(current, total);
                        }
                    }
                } catch (Exception e) {
                    BoyiaLog.e(TAG, "upload write error", e);
                }
            }
        };
    }
}
