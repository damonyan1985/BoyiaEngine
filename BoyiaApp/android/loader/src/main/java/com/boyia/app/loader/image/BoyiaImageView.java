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

    public BoyiaImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public BoyiaImageView(Context context) {
        this(context, 0);
    }

    public BoyiaImageView(Context context, int radius) {
        this(context, null);
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

    private Paint getPaint() {
        Paint paint = new Paint();
        paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.DST_OUT));
        paint.setStyle(Paint.Style.FILL);
        paint.setAntiAlias(true);

        return paint;
    }

    private Path preparePathToDraw(Path roundedPath) {
        Path pathToDraw = new Path();
        pathToDraw.reset();
        pathToDraw.addRect(0, 0, getWidth(), getHeight(), Path.Direction.CW);
        pathToDraw.op(roundedPath, Path.Op.DIFFERENCE);
        return pathToDraw;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (mRadius == 0) {
            super.onDraw(canvas);
            return;
        }

        canvas.saveLayer(null, null, Canvas.ALL_SAVE_FLAG);
        super.onDraw(canvas);

        Paint paint = getPaint();
        RectF rect = new RectF();
        rect.left = 0;
        rect.top = 0;
        rect.right = getWidth();
        rect.bottom = getHeight();
        Path roundedPath = new Path();
        roundedPath.reset();
        roundedPath.addRoundRect(rect, mRadius, mRadius, Path.Direction.CW);

        Path pathToDraw = preparePathToDraw(roundedPath);
        canvas.drawPath(pathToDraw, paint);
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
