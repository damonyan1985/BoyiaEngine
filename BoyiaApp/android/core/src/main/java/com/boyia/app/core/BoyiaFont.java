package com.boyia.app.core;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;

import android.graphics.Paint;
import android.graphics.Paint.FontMetrics;
import android.graphics.Rect;

import java.util.ArrayList;
import java.util.List;

public class BoyiaFont {
    private Paint mFontPaint = null;
    private List<LineText> mList = null;

    public BoyiaFont() {
        mFontPaint = new Paint();
        mList = new ArrayList<LineText>();
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

    public int calcTextLine(String text, int maxWidth, int fontSize) {
        mFontPaint.setTextSize(fontSize);
        mList.clear();

        //char[] charArray = text.toCharArray();
        int currentLineWidth = 0;
        int maxLineWidth = 0;
        //StringBuilder builder = new StringBuilder();
        int start = 0;
        int end = 0;
        BoyiaLog.d("libboyia", "java text=" + text);
        Rect rect = new Rect();
        //for (int index = 0; index < text.length(); ++index) {
        int index = 0;
        while (index < text.length()) {
            end = index+1;
            String subText = text.substring(start, end);
            //mFontPaint.getTextBounds(subText, 0, subText.length(), rect);
            int width = (int)Math.ceil(mFontPaint.measureText(subText));
            //int width = BoyiaUtils.getFontWidth(mFontPaint, charArray, index);
            //BoyiaLog.d("libboyia", "text=" + charArray[index] + " and width=" + width);
//            if (currentLineWidth + width <= maxWidth) {
//                builder.append(charArray[index]);
//                currentLineWidth += width;
//            } else {
//                maxLineWidth = maxLineWidth < currentLineWidth ?
//                        currentLineWidth : maxLineWidth;
//                mList.add(new LineText(builder.toString(), currentLineWidth));
//                currentLineWidth = 0;
//                builder.delete(0, builder.length());
//            }
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

    private static class LineText {
        public String mText;
        public int mWidth;

        public LineText(String text, int width) {
            mText = text;
            mWidth = width;
        }
    }
}
