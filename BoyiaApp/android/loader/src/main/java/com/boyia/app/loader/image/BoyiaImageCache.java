package com.boyia.app.loader.image;

import android.graphics.Bitmap;
import android.os.Build;
import android.util.LruCache;

public class BoyiaImageCache extends LruCache<String, Bitmap> {
    public BoyiaImageCache(int maxSize) {
        super(maxSize);
    }

    @Override
    protected int sizeOf(String key, Bitmap value) {
        return getBitmapSize(value) / 1024;
    }

    public static int getBitmapSize(Bitmap bitmap) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            return bitmap.getAllocationByteCount();
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR1) {
            return bitmap.getByteCount();
        }

        return bitmap.getRowBytes() * bitmap.getHeight();
    }
}
