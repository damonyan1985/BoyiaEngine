package com.boyia.app.loader.image;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

// bitmap池，复用bitmap
public class BoyiaBitmapPool {
    // 利用老bitmap
    private Bitmap getBitmapWithOld(Bitmap oldBitmap, byte[] data) {
        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeByteArray(data, 0, data.length, options);
        if (canUseOldBitmap(oldBitmap, options)) {
            options.inMutable = true;
            options.inBitmap = oldBitmap;
        }
        options.inJustDecodeBounds = false;
        return BitmapFactory.decodeByteArray(data, 0, data.length, options);
    }

    private boolean canUseOldBitmap(Bitmap oldBitmap, BitmapFactory.Options options) {
        int width = options.outWidth / Math.max(options.inSampleSize, 1);
        int height = options.outHeight / Math.max(options.inSampleSize, 1);
        int byteSize = width * height * getBytesPerPixel(oldBitmap.getConfig());

        return byteSize <= oldBitmap.getAllocationByteCount();
    }

    private int getBytesPerPixel(Bitmap.Config config) {
        switch (config) {
            case ALPHA_8:
                return 1;
            case RGB_565:
            case ARGB_4444:
                return 2;
            default:
                return 4;
        }
    }
}
