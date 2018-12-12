package com.boyia.app.core;

import com.boyia.app.utils.BoyiaLog;
import com.boyia.app.utils.BoyiaUtils;

import android.graphics.Paint;
import android.graphics.Paint.FontMetrics;

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
        return (int)(fontMetrics.bottom - fontMetrics.top);
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

        char[] charArray = text.toCharArray();
        int length = charArray.length;
        int currentLineWidth = 0;
        int maxLineWidth = 0;
        StringBuilder builder = new StringBuilder();
        BoyiaLog.d("libboyia", "java text="+text);
        for (int index = 0; index < length; ++index) {
            int width = BoyiaUtils.getFontWidth(mFontPaint, charArray, index);
            BoyiaLog.d("libboyia", "text="+charArray[index] +" and width="+width);
            if (currentLineWidth + width <= maxWidth) {
                builder.append(charArray[index]);
                currentLineWidth += width;
            } else {
                if (maxLineWidth < currentLineWidth) {
                    maxLineWidth = currentLineWidth;
                }
                mList.add(new LineText(builder.toString(), currentLineWidth));
                currentLineWidth = 0;
                builder.delete(0, builder.length());
            }
        }

        if (currentLineWidth > 0) {
            if (maxLineWidth < currentLineWidth) {
                maxLineWidth = currentLineWidth;
            }

            mList.add(new LineText(builder.toString(), currentLineWidth));
            builder.delete(0, builder.length());
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
