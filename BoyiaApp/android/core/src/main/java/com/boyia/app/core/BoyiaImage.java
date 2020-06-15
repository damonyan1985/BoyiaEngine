package com.boyia.app.core;

import com.boyia.app.loader.image.IBoyiaImage;
import com.boyia.app.loader.image.BoyiaImager;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.common.utils.GraphicsConst;

import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;

/*
 * BoyiaImage
 * @Author Yan bo
 * @Time 2018-8-31
 * @Copyright Reserved
 * @Descrption Android 2D Bitmap Interface Implements
 */
public class BoyiaImage implements IBoyiaImage {
    private static final String TAG = "BoyiaImage";
    private Bitmap mBitmap = null;
    private String mUrl = null;
    private long mImagePtr = 0;
    private int mWidth = 0;
    private int mHeight = 0;

    public BoyiaImage() {
    }

    // Call by native image
    public void loadImage(String url, long imagePtr, int width, int height) {
        mImagePtr = imagePtr;
        mWidth = width;
        mHeight = height;
        mUrl = url;
        BoyiaImager.loadImage(url, this);
    }

    // 得到图片字节流 数组大小
    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    // Call by Native
    public void recycle() {
        if (mBitmap != null) {
            mBitmap.recycle();
            mBitmap = null;
        }
    }

    public Bitmap getBitmap() {
        return mBitmap;
    }

    // Call by native image, Draw TextView
    public void drawText(String text, int width, int height, int textSize, int textColor, int textStyle) {
        BoyiaLog.d(TAG, "BoyiaApp bitmap drawText=" + text + " width=" + width + " height=" + height);

        mBitmap = Bitmap.createBitmap(width, height, Config.ARGB_8888);

        Canvas canvas = new Canvas(mBitmap);
        canvas.drawColor(Color.argb(0, 0, 0, 0));

        Paint paint = new Paint();
        paint.setAntiAlias(true);
        switch (textStyle) {
            case 0:
                break;
            case GraphicsConst.PEN_STYLE_ITALIC:
                paint.setTextSkewX(-0.5f);
                break;
            case GraphicsConst.PEN_STYLE_BOLD:
                paint.setFakeBoldText(true);
                break;
            case GraphicsConst.PEN_STYLE_UNDERLINE:
                paint.setUnderlineText(true);
                break;
        }

        paint.setTextSize(textSize);
        paint.setColor(Color.argb(Color.alpha(textColor), Color.red(textColor), Color.green(textColor),
                Color.blue(textColor)));

        BoyiaLog.d(TAG, "gl drawText=" + text);
        BoyiaUtils.drawText(text, new Rect(0, 0, width, height), GraphicsConst.TEXT_ALIGN_LEFT, canvas, paint);
    }

    @Override
    public void setImageURL(String url) {
        mUrl = url;
    }

    @Override
    public String getImageURL() {
        return mUrl;
    }

    @Override
    public void setImage(Bitmap bitmap) {
        mBitmap = bitmap;
        BoyiaUIView.nativeImageLoaded(mImagePtr);
    }
}
