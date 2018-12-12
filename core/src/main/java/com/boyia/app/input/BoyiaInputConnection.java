package com.boyia.app.input;

import com.boyia.app.core.BoyiaUIView;
import com.boyia.app.utils.BoyiaLog;
import com.boyia.app.utils.BoyiaUtils;

import android.view.View;
import android.view.inputmethod.BaseInputConnection;



public class BoyiaInputConnection extends BaseInputConnection {
    private BoyiaUIView mView;
    private String mText;
	
	public BoyiaInputConnection(BoyiaUIView view, boolean fullEditor) {
		super(view, fullEditor);
		mView = view;
		mText = "";
	}

	@Override
	public boolean commitText(CharSequence text, int newCursorPosition) {
		//BoyiaUtils.showToast("BoyiaInputConnection inputtext="+text);
		mText += text;
		mView.setInputText(mText);
		//BoyiaUtils.showToast(mText);
		return true;
	}

	public void resetCommitText() {
		//BoyiaUtils.showToast("resetCommitText mText="+mText);
		mText = "";
	}
	
	public void deleteCommitText() {
		if (mText.length() > 0) {
			mText = mText.substring(0, mText.length() - 1);
			BoyiaUtils.showToast("BoyiaInputConnection deleteCommitText="+mText);
			mView.setInputText(mText);
		}
		
	}
}
