package com.boyia.app.input;

import com.boyia.app.core.BoyiaUIView;
import com.boyia.app.common.utils.BoyiaUtils;
import android.view.inputmethod.BaseInputConnection;

public class BoyiaInputConnection extends BaseInputConnection {
    private BoyiaUIView mView;
    //private String mText;
	private StringBuilder mBuilder;
	
	public BoyiaInputConnection(BoyiaUIView view, boolean fullEditor) {
		super(view, fullEditor);
		mView = view;
		mBuilder = new StringBuilder();
	}

	@Override
	public boolean commitText(CharSequence text, int newCursorPosition) {
		//BoyiaUtils.showToast("BoyiaInputConnection inputtext="+text);
		mBuilder.append(text);
		mView.setInputText(mBuilder.toString());
		//BoyiaUtils.showToast(mText);
		return true;
	}

	public void resetCommitText(final String text) {
		//BoyiaUtils.showToast("resetCommitText mText="+mText);
		mBuilder.delete(0, mBuilder.length());
		if (!BoyiaUtils.isTextEmpty(text)) {
            mBuilder.append(text);
        }
	}
	
	public void deleteCommitText() {
		if (mBuilder.length() > 0) {
			mBuilder = mBuilder.deleteCharAt(mBuilder.length() - 1);
			BoyiaUtils.showToast("BoyiaInputConnection deleteCommitText="+mBuilder.toString());
			mView.setInputText(mBuilder.toString());
		}
		
	}
}
