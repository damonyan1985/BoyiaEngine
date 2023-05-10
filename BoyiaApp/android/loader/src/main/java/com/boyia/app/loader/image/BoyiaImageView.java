package com.boyia.app.loader.image;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.widget.ImageView;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.loader.mue.MainScheduler;

/*
 * BoyiaImageView
 * @Author yanbo.boyia
 * @Time 2014-5-4
 * @Copyright Reserved
 * @Descrption BoyiaImageView
 *     Base On Android UI
 */

public class BoyiaImageView extends ImageView implements IBoyiaImage {
    private static final String TAG = "BoyiaImageView";
    private int mRadius = 0;
    private RectF mImageRect = new RectF();
    private Path mRoundedPath = new Path();
    private Path mDrawPath = new Path();
    private Paint mPaint = new Paint();

    public BoyiaImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public BoyiaImageView(Context context) {
        this(context, 0);
    }

    public BoyiaImageView(Context context, int radius) {
        this(context, null);
        initImageView(radius);
    }

    private void initImageView(int radius) {
        mRadius = radius;
        // Auto fit the size
        setAdjustViewBounds(true);
        setScaleType(ScaleType.FIT_XY);
    }

    public BoyiaImageView(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    @Override
    public void setImageURL(String tag) {
        setTag(tag);
    }

    @Override
    public String getImageURL() {
        return (String) getTag();
    }

    @Override
    public void setImage(final Bitmap bm) {
        BoyiaLog.d(TAG, "BoyiaImageView setImage");
        MainScheduler.mainScheduler().sendJob(() -> setImageBitmap(bm));
    }

    @Override
    public int getImageWidth() {
        return getWidth();
    }

    @Override
    public int getImageHeight() {
        return getHeight();
    }

    public void releaseImageViewResouce() {
        Drawable drawable = getDrawable();
        if (drawable != null && drawable instanceof BitmapDrawable) {
            BitmapDrawable bitmapDrawable = (BitmapDrawable) drawable;
            Bitmap bitmap = bitmapDrawable.getBitmap();
            if (bitmap != null && !bitmap.isRecycled()) {
                bitmap.recycle();
            }
        }
    }

    public void setRadius(int radius) {
        setRadius(radius, false);
    }

    public void setRadius(int radius, boolean draw) {
        mRadius = radius;
        invalidate();
    }

    private void resetPaint() {
        mPaint.reset();
        mPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_OUT));
        mPaint.setStyle(Paint.Style.FILL);
        mPaint.setAntiAlias(true);
    }

    private void preparePathToDraw(Path roundedPath) {
        mDrawPath.reset();
        mDrawPath.addRect(0, 0, getWidth(), getHeight(), Path.Direction.CW);
        mDrawPath.op(roundedPath, Path.Op.DIFFERENCE);
    }

    /**
     * 优化，将onDraw中所有new对象的地方去掉，换成内部成员
     * ondraw操作频繁，不适合在函数中创建对象
     * @param canvas the canvas on which the background will be drawn
     */
    @Override
    protected void onDraw(Canvas canvas) {
        if (mRadius == 0) {
            super.onDraw(canvas);
            return;
        }

        canvas.saveLayer(null, null, Canvas.ALL_SAVE_FLAG);
        super.onDraw(canvas);

        resetPaint();

        mImageRect.left = 0;
        mImageRect.top = 0;
        mImageRect.right = getWidth();
        mImageRect.bottom = getHeight();

        mRoundedPath.reset();
        mRoundedPath.addRoundRect(mImageRect, mRadius, mRadius, Path.Direction.CW);

        preparePathToDraw(mRoundedPath);
        canvas.drawPath(mDrawPath, mPaint);
        canvas.restore();
    }

    /**
     * 封装一个方法，方便调用
     * @param url
     */
    public void load(String url) {
        BoyiaImager.loadImage(url, this);
    }
}
