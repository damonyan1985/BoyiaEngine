package com.boyia.app.core;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.Paint.FontMetrics;
import android.graphics.Point;
import android.graphics.Rect;

/*
 * BoyiaGraphics
 * @Author Yan bo
 * @Time 2014-5-4
 * @Copyright Reserved
 * @Descrption Android 2D Graphic Interface Implements
 */
public class BoyiaGraphics {
    private static final String TAG = "BoyiaGraphics";
    public static final int NULL_BRUSH = 1;
    public static final int SOLID_BRUSH = 2;

    public static final int NULL_PEN = 1;
    public static final int SOLID_PEN = 2;

    public static final int PEN_STYLE_ITALIC = 1;
    public static final int PEN_STYLE_BOLD = 2;
    public static final int PEN_STYLE_UNDERLINE = 3;
    public static final int PEN_STYLE_SIZE_LARGE = 4;
    public static final int PEN_STYLE_SIZE_MEDIUM = 5;
    public static final int PEN_STYLE_SIZE_SMALL = 6;

    public static final int TEXT_ALIGN_LEFT = 1;
    public static final int TEXT_ALIGN_CENTER = 2;
    public static final int TEXT_ALIGN_RIGHT = 3;

    private Paint mPen = null;
    private Paint mBrush = null;
    private Canvas mCanvas = null;

    private int mPenStyle = SOLID_PEN;

    public BoyiaGraphics(Canvas canvas) {
        mPen = new Paint();
        mPen.setAntiAlias(true);
        mBrush = new Paint();
        mBrush.setAntiAlias(true);
        mCanvas = canvas;
    }

    public BoyiaGraphics() {
        mPen = new Paint();
        mPen.setAntiAlias(true);
        mBrush = new Paint();
        mBrush.setAntiAlias(true);
    }

    public void setCanvas(Canvas canvas) {
        mCanvas = canvas;
    }

    public void drawLine(Point p1, Point p2) {
        mCanvas.drawLine(p1.x, p1.y, p2.x, p2.y, mPen);
    }

    public void drawLine(int x1, int y1, int x2, int y2) {
        mCanvas.drawLine(x1, y1, x2, y2, mPen);
    }

    public void drawRect(Rect r) {
        mCanvas.drawRect(r, mBrush);
    }

    public void drawRect(int l, int t, int w, int h) {
        BoyiaLog.d(TAG, "java drawRect" + l + t + w + h);
        mCanvas.drawRect(l, t, w + l, t + h, mBrush);
    }

    public void drawText(String s, int x, int y) {
        BoyiaLog.d(TAG, "java drawText " + s);
        FontMetrics fontMetrics = mPen.getFontMetrics();
        float baseLine = 0;
        baseLine -= fontMetrics.ascent;
        float newY = y + baseLine;
        if (mCanvas != null) {
            mCanvas.drawText(s, x, newY, mPen);
        }
    }

    public void drawText(String s, Point p) {
        drawText(s, p.x, p.y);
    }

    public void drawText(String s, int left, int top, int right, int bottom, int align) {
        drawText(s, new Rect(left, top, right, bottom), align);
    }

    private void drawText(String s, Rect r, int align) {
        BoyiaUtils.drawText(s, r, align, mCanvas, mPen);
    }

    public void setPenColor(int c) {
        int color = Color.rgb(Color.red(c), Color.green(c), Color.blue(c));
        mPen.setColor(color);
    }

    public void setBrushColor(int color) {
        BoyiaLog.d(TAG, "java color=" + Integer.toHexString(color));
        int brushColor = Color.rgb(Color.red(color), Color.green(color),
                Color.blue(color));
        mBrush.setColor(brushColor);
    }

    public void setFont(int fontSize, int fontStyle) {
        mPen.setTextSize((float) fontSize);
        setFontStyle(fontStyle);
    }

    public void setFontWeight(int size) {
        mPen.setStrokeWidth(size);
    }

    public void setPenStyle(int style) {
        mPenStyle = style;
    }

    public void setBrushStyle(int style) {
        switch (style) {
            case NULL_BRUSH:
                mBrush.setStyle(Paint.Style.STROKE);
                mBrush.setStrokeWidth(mPen.getStrokeWidth());
                break;
            case SOLID_BRUSH:
                mBrush.setStyle(Paint.Style.FILL);
                break;
        }
    }

    private void setFontStyle(int style) {
        switch (style) {
            case 0:
                break;
            case PEN_STYLE_ITALIC:
                mPen.setTextSkewX(-0.5f);
                break;
            case PEN_STYLE_BOLD:
                mPen.setFakeBoldText(true);
                break;
            case PEN_STYLE_UNDERLINE:
                mPen.setUnderlineText(true);
                break;
            case PEN_STYLE_SIZE_LARGE:
                mPen.setTextSize(60);
                break;
            case PEN_STYLE_SIZE_MEDIUM:
                mPen.setTextSize(40);
                break;
            case PEN_STYLE_SIZE_SMALL:
                mPen.setTextSize(24);
                break;
        }
    }

    public void drawBitmap(BoyiaImage bitmap,
                           int left,
                           int top,
                           int width,
                           int height) {
        BoyiaLog.d(TAG, "BoyiaGraphicsContext.drawBitmap");
        if (bitmap.getBitmap() == null) {
            return;
        }

        Matrix matrix = new Matrix();
        BoyiaLog.d(TAG, "left=" + left + " " + "width=" + width + " " + "height=" + height + " "
                + "bitmap width=" + bitmap.getImageWidth() + " " + "bitmap height=" + bitmap.getImageHeight());
        matrix.postScale(1.0f * width / bitmap.getImageWidth(),
                1.0f * height / bitmap.getImageHeight());

        Bitmap dstbmp = Bitmap.createBitmap(bitmap.getBitmap(), 0, 0,
                bitmap.getImageWidth(), bitmap.getImageWidth(), matrix, true);

        mCanvas.drawBitmap(dstbmp, left, top, null);
    }

    public void drawBitmap(Bitmap bitmap,
                           int left,
                           int top,
                           int width,
                           int height,
                           int tmp) {
        BoyiaLog.d(TAG, "BoyiaGraphicsContext.drawBitmap");
        if (bitmap == null) {
            return;
        }

        Matrix matrix = new Matrix();
        BoyiaLog.d(TAG, "left=" + left + " " + "width=" + width + " " + "height=" + height + " "
                + "bitmap width=" + bitmap.getWidth() + " " + "bitmap height=" + bitmap.getHeight());
        matrix.postScale(1.0f * width / bitmap.getWidth(),
                1.0f * height / bitmap.getHeight());

        Bitmap dstbmp = Bitmap.createBitmap(bitmap, 0, 0,
                bitmap.getWidth(), bitmap.getHeight(), matrix, true);

        mCanvas.drawBitmap(dstbmp, left, top, null);
    }
}
