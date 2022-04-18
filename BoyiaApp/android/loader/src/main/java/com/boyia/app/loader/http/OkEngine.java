package com.boyia.app.loader.http;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.TimeUnit;

import okhttp3.Call;
import okhttp3.OkHttpClient;
import okhttp3.RequestBody;

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
                .connectTimeout(600, TimeUnit.SECONDS)
                .readTimeout(600, TimeUnit.SECONDS)
                .sslSocketFactory(info.mFactory, info.mTrustManager)
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
}
