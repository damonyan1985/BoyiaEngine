package com.boyia.app.core;

import com.boyia.app.http.HttpResponseData;
import com.boyia.app.http.HttpUtil;
import com.boyia.app.job.JobBase;
import com.boyia.app.job.JobScheduler;
import com.boyia.app.loader.ILoadImage;
import com.boyia.app.loader.BoyiaImageManager;
import com.boyia.app.utils.BoyiaLog;
import com.boyia.app.utils.BoyiaUtils;

import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;

/*
 * BoyiaBitmap
 * @Author Yan bo
 * @Time 2018-8-31
 * @Copyright Reserved
 * @Descrption Android 2D Bitmap Interface Implements
 */
public class BoyiaBitmap implements ILoadImage {

    private static final String TAG = "BoyiaBitmap";
    private Bitmap mBitmap = null;
    private String mUrl = null;
    private long   mImagePtr = 0;
    private int    mWidth = 0;
    private int    mHeight = 0;

    public BoyiaBitmap() {
    }

    public void loadImage(String url, long imagePtr, int width, int height) {
    	mImagePtr = imagePtr;
    	mWidth = width;
    	mHeight = height;
    	mUrl = url;
    	asyncLoadImage(url);
    }
    
    public void asyncLoadImage(String url) {
    	BoyiaImageManager.getInstance().loadImage(url, this);
    }

    // 得到图片字节流 数组大小
    public int getWidth() {
    	return mWidth;
    }

    public int getHeight() {
    	return mHeight;
    }

    public void recycle() {
        if (mBitmap != null) {
            mBitmap.recycle();
            mBitmap = null;
        }
    }

    public Bitmap getBitmap() {
        return mBitmap;
    }

    // 绘制文字部分
    public void drawText(String text,
                         int width, int height,
                         int align, int textSize,
                         int textColor,
                         int textStyle,
                         int bgColor) {
    	BoyiaLog.d("BoyiaApp", "BoyiaApp bitmap drawText="+text + " width="+width+ " height="+height);

        mBitmap = Bitmap.createBitmap(
                width,
                height,
                Config.ARGB_8888);

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
        paint.setColor(Color.argb(
                Color.alpha(textColor),
                Color.red(textColor),
                Color.green(textColor),
                Color.blue(textColor)));

        BoyiaLog.d(TAG, "gl drawText=" + text);
        BoyiaUtils.drawText(text,
                new Rect(0, 0, width, height),
                align,
                canvas,
                paint);
        
        //mBitmap.recycle();
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
