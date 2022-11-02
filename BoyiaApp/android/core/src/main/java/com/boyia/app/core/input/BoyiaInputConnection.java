package com.boyia.app.core.input;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.BoyiaBridge;
import com.boyia.app.core.BoyiaView;
import com.boyia.app.common.utils.BoyiaUtils;

import android.os.Bundle;
import android.text.InputType;
import android.view.KeyEvent;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputContentInfo;

public class BoyiaInputConnection extends BaseInputConnection {
    private static final String TAG = "BoyiaInputConnection";
    private BoyiaView mView;
    private StringBuilder mBuilder;
    private int mCursorIndex;
    private EditorInfo mEditorInfo;

    public BoyiaInputConnection(BoyiaView view, EditorInfo info) {
        super(view, false);
        mView = view;
        mBuilder = new StringBuilder();
        mEditorInfo = info;
        // gboard必须设置EditorInfo
        info.inputType = InputType.TYPE_CLASS_TEXT;
        info.imeOptions = EditorInfo.IME_ACTION_NEXT;
    }

    /**
     * 输入法输入时会回调
     * @param text
     * @param newCursorPosition
     * @return
     */
    @Override
    public boolean commitText(CharSequence text, int newCursorPosition) {
        if (mCursorIndex > mBuilder.length()) {
            mCursorIndex = mBuilder.length();
        }

        if (mCursorIndex == mBuilder.length()) {
            mBuilder.append(text);
        } else {
            mBuilder.insert(mCursorIndex, text);
        }

        mCursorIndex += text.length();
        String inputText = mBuilder.toString();
        BoyiaLog.d(TAG, "commitText inputText=" + inputText);
        mView.setInputText(inputText, mCursorIndex);
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

    @Override
    public boolean commitContent(InputContentInfo inputContentInfo, int flags, Bundle opts) {
        BoyiaLog.d(TAG, "BoyiaInputConnection call commitContent");
        return super.commitContent(inputContentInfo, flags, opts);
    }

    @Override
    public boolean finishComposingText() {
        BoyiaLog.d(TAG, "BoyiaInputConnection call finishComposingText");
        return super.finishComposingText();
    }

    @Override
    public boolean sendKeyEvent(KeyEvent event) {
        BoyiaLog.d(TAG, "BoyiaInputConnection call sendKeyEvent");
        return super.sendKeyEvent(event);
    }

    @Override
    public boolean setComposingText(CharSequence text, int newCursorPosition) {
        BoyiaLog.d(TAG, "BoyiaInputConnection call setComposingText text=" + text);
        return super.setComposingText(text, newCursorPosition);
    }
}
