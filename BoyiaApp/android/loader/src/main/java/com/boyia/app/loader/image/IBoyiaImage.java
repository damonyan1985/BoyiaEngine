package com.boyia.app.loader.image;

import android.graphics.Bitmap;

/*
 * IBoyiaImage
 * @Author yanbo.boyia
 * @Time 2014-5-4
 * @Copyright Reserved
 * @Descrption ITargetImage
 * All network image interface
 */

public interface IBoyiaImage {
    void setImageURL(String url);

    String getImageURL();

    void setImage(Bitmap bitmap);

    int getImageWidth();

    int getImageHeight();
}
