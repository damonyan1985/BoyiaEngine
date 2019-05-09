package com.boyia.app.common.http;

import java.util.Map;

import okhttp3.FormBody;
import okhttp3.Headers;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import java.util.List;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.Set;

public class OKHTTPEngine extends BaseHTTPEngine {
    @Override
    public void stop() {
    }

    @Override
    public HTTPResponse getStream(HTTPRequest request) {
        HTTPResponse response = getLocalData(request.mUrl);
        if (null == response) {
            response = new HTTPResponse();
            Request.Builder builder = setHeader(request);
            OkHttpClient client = new OkHttpClient();

            switch (request.mMethod) {
                case HTTPUtil.HTTP_GET_METHOD:
                    builder.get();
                    break;
                case HTTPUtil.HTTP_POST_METHOD: {
                    FormBody.Builder formBuilder = new FormBody.Builder();
                    if (request.mPostParams != null && request.mPostParams.size() > 0) {
                        for (Map.Entry<String, String> entry : request.mPostParams
                                .entrySet()) {
                            formBuilder.add(entry.getKey(), entry.getValue());
                        }
                    }
                    builder.post(formBuilder.build());
                }
                    break;
            }

            try {
                Response res = client.newCall(builder.build()).execute();
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
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }

        return response;
    }

    private Request.Builder setHeader(HTTPRequest request) {
        Request.Builder builder = new Request.Builder();
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
