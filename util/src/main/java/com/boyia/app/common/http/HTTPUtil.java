package com.boyia.app.common.http;

/*
 * HttpUtil
 * @Author Yan bo
 * @Time 2018-9-1
 * @Copyright Reserved
 * @Descrption HTTP Resource Base Function
 *  Interface
 */

public class HTTPUtil {
	public static final String TAG = HTTPUtil.class.getSimpleName();
	public static final int HTTP_GET_METHOD = 0;
	public static final int HTTP_POST_METHOD = 1;
	public static final String HTTP_GET_METHOD_STR = "GET";
	public static final String HTTP_POST_METHOD_STR = "POST";
	public static final int HTTP_TIME_OUT = 10 * 1000;
	public static final String HTTP_CHARSET_UTF8 = "utf-8";

	// HTTP协议头部
	public static final String HTTP_HEADER_CONTENT_TYPE = "Content-Type";
	public static final String HTTP_HEADER_CONNECT = "Connection";
	public static final String HTTP_HEADER_CHARSET = "Charset";
	public static final String HTTP_HEADER_CONTENT_LENGTH = "Content-Length";

	public static final String HTTP_HEADER_VALUE_FORM = "application/x-www-form-urlencoded";
	public static final String HTTP_HEADER_VALUE_KEEP = "Keep-Alive";
	
	// HTTP引擎类型
	public static final int OK_ENGINE = 0;
	public static final int URL_ENGINE = 1;
	
	public static HTTPResponse getFileStream(String url) {
		return getFileStream(new HTTPRequest(
				url, 
				HTTPUtil.HTTP_GET_METHOD,
				null, null));
	}
	
	public static HTTPResponse getFileStream(HTTPRequest request) {
		return BaseHTTPEngine.getHttpEngine(HTTPUtil.URL_ENGINE).getStream(request);
	}

	public static BaseHTTPEngine createHttpEngine() {
		return BaseHTTPEngine.getHttpEngine(HTTPUtil.OK_ENGINE);
	}
}
