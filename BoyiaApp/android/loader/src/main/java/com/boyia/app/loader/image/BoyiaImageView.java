package com.boyia.app.loader.image;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.widget.ImageView;

import com.boyia.app.loader.jober.MainScheduler;

/*
 * BoyiaImageView
 * @Author yanbo.boyia
 * @Time 2014-5-4
 * @Copyright Reserved
 * @Descrption BoyiaImageView
 *     Base On Android UI
 */

public class BoyiaImageView extends ImageView implements IBoyiaImage {
    public BoyiaImageView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    public BoyiaImageView(Context context) {
        this(context, null);
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
        MainScheduler.mainScheduler().sendJob(() -> setImageBitmap(bm));
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
}
