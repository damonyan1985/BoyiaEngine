package com.boyia.app.loader.image;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.loader.BoyiaLoader;

import java.lang.ref.WeakReference;
import java.util.List;


public class BoyiaImageHandler {
    public static final String TAG = "BoyiaImageHandler";
    private static final int RETRY_TIME = 3;
    private int mRetryTimes;
    private BoyiaLoader mLoader;
    private String mUrl;

    BoyiaImageHandler(String url) {
        mRetryTimes = RETRY_TIME;
        mUrl = url;
    }

    public void setLoader(BoyiaLoader loader) {
        mLoader = loader;
        mLoader.load(mUrl);
    }

    public void retry() {
        if (mRetryTimes > 0) {
            mRetryTimes--;
            mLoader.load(mUrl);
        } else {
            mLoader = null;
            BoyiaImager.getInstance().removeLoadImage(mUrl);
        }
    }

    public void handleRedirect(String url) {
        mLoader.load(url);
    }

    public void decode(byte[] data) {
        try {
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inJustDecodeBounds = true;
            BitmapFactory.decodeByteArray(data, 0,
                    data.length, options);
            options.inJustDecodeBounds = false;

            List<WeakReference<IBoyiaImage>> imageList = BoyiaImager.getInstance()
                    .getImageList(mUrl);

            BoyiaLog.d(TAG, "BoyiaImageHandler decoding imageList = " + imageList + "，mUrl=" + mUrl);
            if (imageList != null && imageList.size() > 0) {
                synchronized (imageList) {
                    int width = options.outWidth;
                    Bitmap bitmap = null;
                    for (WeakReference<IBoyiaImage> loadImageRef : imageList) {
                        IBoyiaImage loadImage = loadImageRef.get();
                        if (loadImage == null) {
                            continue;
                        }

                        if (width != loadImage.getImageWidth()) {
                            options.inSampleSize = loadImage.getImageWidth() == 0 ? 1 : options.outWidth / loadImage.getImageWidth();
                            bitmap = BitmapFactory.decodeByteArray(data, 0, data.length, options);
                            BoyiaImager.getInstance().putBitmapCache(mUrl, bitmap);
                            width = loadImage.getImageWidth();
                        }

                        String url = loadImage.getImageURL();
                        BoyiaLog.d(TAG, "BoyiaImageHandler decode url = " + url + ", mUrl=" + mUrl);
                        if (!BoyiaUtils.isTextEmpty(url) && url.equals(mUrl) && bitmap != null) {
                            loadImage.setImage(bitmap);
                            BoyiaLog.d(TAG, "Decode data in native");
                        }
                    }
                }
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    public void release() {
        // Release loader reference
        mLoader = null;
        // Remove url reference in map
        BoyiaImager.getInstance().removeLoadImage(mUrl);
    }

    public void cacheDisk(byte[] data) {
        BoyiaImager.getInstance().cacheDisk(mUrl, data);
    }
}
