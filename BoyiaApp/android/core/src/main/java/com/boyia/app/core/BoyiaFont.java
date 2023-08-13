package com.boyia.app.core;

import com.boyia.app.common.utils.BoyiaLog;

import android.graphics.Paint;
import android.graphics.Paint.FontMetrics;

import java.util.ArrayList;
import java.util.List;

public class BoyiaFont {
    private Paint mFontPaint;
    private List<LineText> mList;

    public BoyiaFont() {
        mFontPaint = new Paint();
        mList = new ArrayList<>();
    }

    public int getFontWidth(String ch, int size) {
        mFontPaint.setTextSize(size);
        return (int) mFontPaint.measureText(ch);
    }

    public int getFontHeight(int size) {
        BoyiaLog.d("BoyiaFont", "getFontHeight");
        mFontPaint.setTextSize(size);
        FontMetrics fontMetrics = mFontPaint.getFontMetrics();
        return (int) (fontMetrics.bottom - fontMetrics.top);
    }

    public int getTextWidth(String text, int size) {
        return getFontWidth(text, size);
    }

    public int getLineSize() {
        if (mList != null) {
            return mList.size();
        }

        return 0;
    }

    public String getLineText(int index) {
        return mList.get(index).mText;
    }

    public int getLineWidth(int index) {
        return mList.get(index).mWidth;
    }

    /**
     * 分割字符串为多行
     * @param text
     * @param maxWidth
     * @param fontSize
     * @return
     */
    public int calcTextLine(String text, int maxWidth, int fontSize) {
        mFontPaint.setTextSize(fontSize);
        mList.clear();

        int currentLineWidth = 0;
        int maxLineWidth = 0;
        int start = 0;
        int end = 0;
        BoyiaLog.d("libboyia", "java text=" + text);
        int index = 0;
        while (index < text.length()) {
            end = index+1;
            String subText = text.substring(start, end);
            int width = (int)Math.ceil(mFontPaint.measureText(subText));

            if (width <= maxWidth) {
                currentLineWidth = width;
                ++index;
            } else {
                maxLineWidth = maxLineWidth < currentLineWidth ?
                        currentLineWidth : maxLineWidth;

                String resultText = text.substring(start, end-1);
                mList.add(new LineText(resultText, currentLineWidth));
                currentLineWidth = 0;
                start = index;
            }
        }

        if (currentLineWidth > 0) {
            String resultText = text.substring(start, end);
            maxLineWidth = maxLineWidth < currentLineWidth ?
                    currentLineWidth : maxLineWidth;
            mList.add(new LineText(resultText, currentLineWidth));
        }

        return maxLineWidth;
    }

    /**
     * 根据x坐标计算所在line的索引，x坐标为相对位移
     * @param line
     * @param x
     * @return 返回位置所在的索引
     */
    public int getIndexByOffset(int line, int x) {
        if (mList.size() == 0) {
            return 0;
        }

        String text = mList.get(line).mText;
        int i = 0;
        for (; i < text.length(); i++) {
            String subText = text.substring(0, i+1);
            int width = (int)Math.ceil(mFontPaint.measureText(subText));
            if (x < width) {
                return i;
            }
        }

        return i;
    }

    /**
     * 根据索引找到所在x坐标的相对位移
     * @param line
     * @param index
     * @return
     */
    public int getOffsetByIndex(int line, int index) {
        if (mList.size() == 0) {
            return 0;
        }

        String text = mList.get(line).mText;
        if (index > text.length()) {
            index = text.length();
        }

        String subText = text.substring(0, index);
        return (int)Math.ceil(mFontPaint.measureText(subText));
    }

    private static class LineText {
        public String mText;
        public int mWidth;

        public LineText(String text, int width) {
            mText = text;
            mWidth = width;
        }
    }
}
