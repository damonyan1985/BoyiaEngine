package com.boyia.app.core.input;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.core.BoyiaUIView;

import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

public class BoyiaInputManager {
    private static final String TAG = "BoyiaInputManager";
    private BoyiaUIView mView;
    private long mItem = 0;
    private int mRootViewVisibleHeight = 0;

    public BoyiaInputManager(BoyiaUIView view) {
        mView = view;
        initViewListener();
    }

    private void initViewListener() {
        Activity activity = (Activity) mView.getContext();
        final View rootView = activity.getWindow().getDecorView();
        rootView.getViewTreeObserver().addOnGlobalLayoutListener(() -> {
                Rect rect = new Rect();
                rootView.getWindowVisibleDisplayFrame(rect);
                BoyiaLog.d(
                        TAG,
                        "rect.bottom=" + rect.bottom + " height=" + rootView.getHeight());
                if (mRootViewVisibleHeight == 0) {
                    mRootViewVisibleHeight = rect.height();
                    return;
                }

                if (mRootViewVisibleHeight == rect.height()) {
                    return;
                }

                if (mRootViewVisibleHeight - rect.height() > 200) {
                    // 软键盘弹起
                    //mKeyboardHeight = rootView.getHeight() - rect.bottom;
                    BoyiaLog.d(TAG, "BoyiaInputManager SHOW");
                    BoyiaUIView.nativeOnKeyboardShow(mItem, mRootViewVisibleHeight - rect.height());
                    mRootViewVisibleHeight = rect.height();
                } else if (rect.height() - mRootViewVisibleHeight > 200) {
                    BoyiaLog.d(TAG, "BoyiaInputManager HIDE");
                    BoyiaUIView.nativeOnKeyboardHide(mItem, rect.height() - mRootViewVisibleHeight);
                    mRootViewVisibleHeight = rect.height();
                }
            });
    }

    public void show(final long item, final String text) {
        final Activity context = (Activity) mView.getContext();
        context.runOnUiThread(() -> {
                BoyiaUtils.showToast("show keyboard");
                mItem = item;
                InputMethodManager imm = (InputMethodManager) context.getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.showSoftInput(mView, 0, null);
                mView.resetCommitText(text);
        });
    }

    public void hide() {
        InputMethodManager imm = (InputMethodManager) mView.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(mView.getWindowToken(), 0);
    }

    public long item() {
        return mItem;
    }
}
