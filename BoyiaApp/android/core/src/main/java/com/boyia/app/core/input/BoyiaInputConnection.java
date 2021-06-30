package com.boyia.app.core.input;

import com.boyia.app.core.BoyiaView;
import com.boyia.app.common.utils.BoyiaUtils;

import android.view.inputmethod.BaseInputConnection;

public class BoyiaInputConnection extends BaseInputConnection {
    private BoyiaView mView;
    private StringBuilder mBuilder;

    public BoyiaInputConnection(BoyiaView view, boolean fullEditor) {
        super(view, fullEditor);
        mView = view;
        mBuilder = new StringBuilder();
    }

    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
        mBuilder.append(text);
        mView.setInputText(mBuilder.toString());
        return true;
    }

    public void resetCommitText(final String text) {
        mBuilder.delete(0, mBuilder.length());
        if (!BoyiaUtils.isTextEmpty(text)) {
            mBuilder.append(text);
        }
    }

    public void deleteCommitText() {
        if (mBuilder.length() > 0) {
            mBuilder = mBuilder.deleteCharAt(mBuilder.length() - 1);
            BoyiaUtils.showToast("BoyiaInputConnection deleteCommitText=" + mBuilder.toString());
            mView.setInputText(mBuilder.toString());
        }
    }
}
