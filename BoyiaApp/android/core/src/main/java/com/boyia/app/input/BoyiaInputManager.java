package com.boyia.app.input;

import com.boyia.app.common.base.BaseActivity;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.BoyiaUIView;

import android.app.Activity;
import android.content.Context;
import android.graphics.Rect;
import android.view.View;
import android.view.ViewTreeObserver;
import android.view.inputmethod.InputMethodManager;

public class BoyiaInputManager {
	private static final String TAG = BoyiaInputManager.class.getSimpleName();
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
		rootView.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener() {
			@Override
			public void onGlobalLayout() {
				Rect rect = new Rect();
				rootView.getWindowVisibleDisplayFrame(rect);
				BoyiaLog.d(
						TAG,
						"rect.bottom="+rect.bottom +" height="+rootView.getHeight());
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
			}
		});
	}
    
    public void show(final long item, final String text) {
    	//BoyiaUtils.showToast("重新弹起");
    	BaseActivity context = (BaseActivity) mView.getContext();
    	context.runOnUiThread(new Runnable() {
			@Override
			public void run() {
		    	mItem = item;
		    	InputMethodManager imm = (InputMethodManager) mView.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
		    	imm.showSoftInput(mView, 0, null);
		    	mView.resetCommitText(text);
			}
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
