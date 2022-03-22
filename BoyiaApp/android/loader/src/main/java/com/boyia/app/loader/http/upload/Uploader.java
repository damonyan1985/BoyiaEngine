package com.boyia.app.loader.http.upload;

import com.boyia.app.common.utils.BoyiaLog;

import java.io.File;
import java.io.IOException;

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
     * @param url 上传的服务端地址
     * @param path 文件路径
     * @param listener 上传进度监听
     */
    static void upload(String url, String path, UploadProgressListener listener) {
        File file = new File(path);
        MultipartBody.Builder builder = new MultipartBody.Builder().setType(MultipartBody.FORM);
        builder.addFormDataPart(
                UPLOADER_TYPE_FILE,
                file.getName(),
                createUpload(MultipartBody.FORM, file, listener));

        Request request = new Request.Builder()
                .url(url)
                .post(builder.build())
                .build();

        new OkHttpClient().newCall(request).enqueue(new Callback() {
            @Override
            public void onFailure(Call call, IOException e) {
                if (listener != null) {
                    listener.onError();
                }
            }

            @Override
            public void onResponse(Call call, Response response) throws IOException {
                // 回传上传结果，比如返回上传文件后的地址等信息
                if (listener != null) {
                    listener.onUploaded(response.body().string());
                }
            }
        });
    }

    private static RequestBody createUpload(
            final MediaType type,
            final File file,
            final UploadProgressListener listener) {
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
                    long size = 0;
                    for (long readSize = 0; (readSize = source.read(buffer, UPLOADER_WRITE_SIZE)) != -1;) {
                        sink.write(buffer, readSize);
                        size += readSize;
                        if (listener != null) {
                            listener.onProgress(contentLength(), size);
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
