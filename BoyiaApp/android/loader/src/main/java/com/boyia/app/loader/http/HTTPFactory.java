package com.boyia.app.loader.http;

/*
 * HTTPFactory
 * @Author Boyia
 * @Date 2018-9-1
 * @Copyright Reserved
 * @Descrption HTTP Resource Base Function
 *  Interface
 */
public class HTTPFactory {
    public static final String TAG = "HTTPFactory";
    public static final int HTTP_GET_METHOD = 0;
    public static final int HTTP_POST_METHOD = 1;
    public static final int HTTP_PUT_METHOD = 2;
    // 使用post方式上传文件
    public static final int HTTP_POST_UPLOAD_METHOD = 3;
    // 使用put方式上传文件
    public static final int HTTP_PUT_UPLOAD_METHOD = 4;

    public static final String HTTP_GET_METHOD_STR = "GET";
    public static final String HTTP_POST_METHOD_STR = "POST";
    public static final int HTTP_TIME_OUT = 3 * 1000;
    public static final String HTTP_CHARSET_UTF8 = "utf-8";

    // HTTP协议头部
    public static class HeaderKeys {
        public static final String CONTENT_TYPE = "Content-Type";
        public static final String CONNECT = "Connection";
        public static final String CHARSET = "Charset";
        public static final String CONTENT_LENGTH = "Content-Length";
    }

    public static class HeaderValues {
        public static final String FORM = "application/x-www-form-urlencoded";
        public static final String JSON = "application/json;charset=UTF-8";
        public static final String KEEP = "Keep-Alive";
    }

    // HTTP引擎类型
    public static final int OK_ENGINE = 0;
    public static final int URL_ENGINE = 1;

    public static final int UPLOADER_WRITE_SIZE = 2048;
    public static final String UPLOADER_TYPE_FILE = "file";

    public static Response getResponse(String url) {
        return getResponse(new Request(
                url,
                HTTPFactory.HTTP_GET_METHOD,
                null, null));
    }

    public static Response getResponse(Request request) {
        return BaseEngine.getHttpEngine(HTTPFactory.OK_ENGINE).getResponse(request);
    }

    public static BaseEngine createHttpEngine() {
        return BaseEngine.getHttpEngine(HTTPFactory.OK_ENGINE);
    }
}
