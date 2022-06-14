package com.boyia.app.loader.http.upload;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.ILoadListener;
import com.boyia.app.loader.http.SSLHelper;

import java.io.File;
import java.io.IOException;
import java.util.Map;
import java.util.concurrent.TimeUnit;

import okhttp3.Call;
import okhttp3.Callback;
import okhttp3.MediaType;
import okhttp3.MultipartBody;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import okio.Buffer;
import okio.BufferedSink;
import okio.Okio;
import okio.Source;

/**
 * 上传文件类
 */
public class Uploader {
    private static final String UPLOADER_TAG = "Boyia_Uploader";
    private static final String UPLOADER_TYPE_FILE = "file";
    private static final int UPLOADER_WRITE_SIZE = 2048;

    /**
     * @param req 上传的服务端请求
     * @param listener 上传进度监听
     */
    public static void upload(com.boyia.app.loader.http.Request req, ILoadListener listener, Object msg) {
        File file = new File(req.mPostData);
        MultipartBody.Builder builder = new MultipartBody.Builder().setType(MultipartBody.FORM);
        builder.addFormDataPart(
                UPLOADER_TYPE_FILE,
                file.getName(),
                createUpload(MultipartBody.FORM, file, listener, msg));

        Request.Builder reqBuilder = new Request.Builder();
        if (req.mHeaders != null && req.mHeaders.size() > 0) {
            for (Map.Entry<String, String> entry : req.mHeaders
                    .entrySet()) {
                reqBuilder.addHeader(entry.getKey(), entry.getValue());
            }
        }

        reqBuilder.url(req.mUrl);
        reqBuilder.post(builder.build());

        Request request = reqBuilder.build();

        SSLHelper.SSLInfo info = SSLHelper.getSSLInfo();
        Call call = new OkHttpClient().newBuilder()
                .connectTimeout(600, TimeUnit.SECONDS)
                .readTimeout(600, TimeUnit.SECONDS)
                .sslSocketFactory(info.mFactory, info.mTrustManager)
                .hostnameVerifier((hostname, session) -> true)
                .build()
                .newCall(request);
        try {
            Response response = call.execute();
            BoyiaLog.d(UPLOADER_TAG, "upload response: " + response.body().string());
            if (listener != null) {
                listener.onLoadFinished(msg);
            }
        } catch (Exception e) {
            if (listener != null) {
                listener.onLoadError(e.toString(), msg);
            }
            BoyiaLog.e(UPLOADER_TAG, "call execute error", e);
        }
    }

    private static RequestBody createUpload(
            final MediaType type,
            final File file,
            final ILoadListener listener,
            final Object msg) {
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
                    if (listener != null) {
                        listener.onLoadDataSize(contentLength(), msg);
                    }

                    for (long readSize; (readSize = source.read(buffer, UPLOADER_WRITE_SIZE)) != -1;) {
                        sink.write(buffer, readSize);
                        if (listener != null) {
                            // 只传入当前读取的大小
                            listener.onLoadDataReceive(null, (int) readSize, msg);
                        }
                    }
                } catch (Exception e) {
                    BoyiaLog.e(UPLOADER_TAG, "upload write error", e);
                }
            }
        };
    }

    public interface UploadProgressListener {
        void onProgress(long totalSize, long progress);
        void onError();
        void onUploaded(String result);
    }
}
