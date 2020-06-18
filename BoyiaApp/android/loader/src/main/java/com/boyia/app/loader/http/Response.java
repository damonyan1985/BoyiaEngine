package com.boyia.app.loader.http;

import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Map;

/*
 * Response
 * @Author Boyia
 * @Date 2018-9-1
 * @Copyright Reserved
 */
public class Response {
    protected int mCode;
    protected InputStream mInput;
    protected long mLength;
    protected Map<String, List<String>> mHeaders;
    protected String mError;

    public Response() {
        this(null, 0, null, null);
    }

    public Response(
            InputStream dataStream,
            int length,
            Map<String, List<String>> headers,
            String error) {
        mInput = dataStream;
        mLength = length;
        mHeaders = headers;
        mError = error;
    }

    public InputStream getStream() {
        return mInput;
    }

    public String getError() {
        return mError;
    }

    public String getRedirectUrl() {
        if (mHeaders != null) {
            List<String> locations = mHeaders.get("Location");
            if (locations != null && locations.size() > 0) {
                return locations.get(0);
            }
        }

        return null;
    }

    public boolean isChuncked() {
        if (mHeaders != null) {
            List<String> chuncks = mHeaders.get("Transfer-Encoding");
            if (chuncks != null && chuncks.size() > 0 && chuncks.get(0).equalsIgnoreCase("chuncked")) {
                return true;
            }
        }

        return false;
    }

    public long getLength() {
        return mLength;
    }

    public int read(byte[] buffer) throws IOException {
        if (mInput != null) {
            return mInput.read(buffer);
        }

        return -1;
    }

    public void close() throws IOException {
        if (mInput != null) {
            mInput.close();
        }
    }
}