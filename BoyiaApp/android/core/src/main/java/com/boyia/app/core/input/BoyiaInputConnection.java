package com.boyia.app.core.input;

import com.boyia.app.core.BoyiaBridge;
import com.boyia.app.core.BoyiaView;
import com.boyia.app.common.utils.BoyiaUtils;

import android.view.inputmethod.BaseInputConnection;

public class BoyiaInputConnection extends BaseInputConnection {
    private BoyiaView mView;
    private StringBuilder mBuilder;
    private int mCursorIndex;

    public BoyiaInputConnection(BoyiaView view, boolean fullEditor) {
        super(view, fullEditor);
        mView = view;
        mBuilder = new StringBuilder();
    }

    /**
     * 输入法输入时会回调
     * @param text
     * @param newCursorPosition
     * @return
     */
    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
        //mBuilder.append(text);
        if (mCursorIndex > mBuilder.length()) {
            mCursorIndex = mBuilder.length();
        }

        if (mCursorIndex == mBuilder.length()) {
            mBuilder.append(text);
        } else {
            mBuilder.insert(mCursorIndex, text);
        }

        mCursorIndex += text.length();
        mView.setInputText(mBuilder.toString(), mCursorIndex);
        return true;
    }

    public void resetCommitText(final String text, final int cursor) {
        mCursorIndex = cursor;
        mBuilder.delete(0, mBuilder.length());
        if (!BoyiaUtils.isTextEmpty(text)) {
            mBuilder.append(text);
        }
    }

    public void deleteCommitText() {
        if (mBuilder.length() > 0 && mCursorIndex > 0) {
            //mBuilder = mBuilder.deleteCharAt(mBuilder.length() - 1);
            mBuilder = mBuilder.deleteCharAt(--mCursorIndex);
            BoyiaBridge.showToast("BoyiaInputConnection deleteCommitText=" + mBuilder.toString());
            mView.setInputText(mBuilder.toString(), mCursorIndex);
        }
    }
}
