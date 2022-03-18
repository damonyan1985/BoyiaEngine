package com.boyia.app.core.input;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.core.BoyiaCoreJNI;
import com.boyia.app.core.BoyiaView;
import com.boyia.app.loader.mue.MainScheduler;

import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

import java.lang.ref.WeakReference;

public class BoyiaInputManager {
    private static final String TAG = "BoyiaInputManager";
    private static final int KEYBOARD_DETECT_HEIGHT = 200;
    private WeakReference<BoyiaView> mViewRef;
    private long mItem = 0;
    private int mRootViewVisibleHeight = 0;

    public BoyiaInputManager(BoyiaView view) {
        mViewRef = new WeakReference<>(view);
        initViewListener();
    }

    private void initViewListener() {
        if (mViewRef.get() == null) {
            return;
        }

        Activity activity = (Activity) mViewRef.get().getContext();
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

                if (mRootViewVisibleHeight - rect.height() > KEYBOARD_DETECT_HEIGHT) {
                    // 软键盘弹起
                    //mKeyboardHeight = rootView.getHeight() - rect.bottom;
                    BoyiaLog.d(TAG, "BoyiaInputManager SHOW");
                    BoyiaCoreJNI.nativeOnKeyboardShow(mItem, mRootViewVisibleHeight - rect.height());
                    mRootViewVisibleHeight = rect.height();
                } else if (rect.height() - mRootViewVisibleHeight > KEYBOARD_DETECT_HEIGHT) {
                    BoyiaLog.d(TAG, "BoyiaInputManager HIDE");
                    BoyiaCoreJNI.nativeOnKeyboardHide(mItem, rect.height() - mRootViewVisibleHeight);
                    mRootViewVisibleHeight = rect.height();
                }
            });
    }

    public void show(final long item, final String text) {
        if (mViewRef.get() == null) {
            return;
        }

        MainScheduler.mainScheduler().sendJob(() -> {
            BoyiaUtils.showToast("show keyboard");
            mItem = item;
            InputMethodManager imm = (InputMethodManager) mViewRef.get()
                    .getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
            imm.showSoftInput(mViewRef.get(), 0, null);
            mViewRef.get().resetCommitText(text);
        });
    }

    public void hide() {
        if (mViewRef.get() == null) {
            return;
        }

        InputMethodManager imm = (InputMethodManager) mViewRef.get().getContext()
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        imm.hideSoftInputFromWindow(mViewRef.get().getWindowToken(), 0);
    }

    public long item() {
        return mItem;
    }
}
