package com.boyia.app.input;

import com.boyia.app.base.BaseActivity;
import com.boyia.app.core.BoyiaUIView;
import com.boyia.app.utils.BoyiaUtils;

import android.content.Context;
import android.os.Message;
import android.os.ResultReceiver;
import android.os.Handler;
import android.view.View;
import android.view.inputmethod.InputMethodManager;

public class BoyiaInputManager {
	//private Context mContext;
	private BoyiaUIView mView;
	private ResultReceiver mReceiver = null;
	private long mItem = 0;
    public BoyiaInputManager(Context context, BoyiaUIView view) {
    	//mContext = context;
    	mView = view;
    	mReceiver = new ResultReceiver(new Handler() {
    		@Override
    		public void handleMessage(Message msg) {
    		
    		}
    	});
    }
    
    public void show(final long item) {
    	//BoyiaUtils.showToast("重新弹起");
    	BaseActivity context = (BaseActivity) mView.getContext();
    	context.runOnUiThread(new Runnable() {

			@Override
			public void run() {
		    	mItem = item;
		    	InputMethodManager imm = (InputMethodManager) mView.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
		    	//imm.toggleSoftInput(0, InputMethodManager.HIDE_NOT_ALWAYS);
		    	imm.showSoftInput(mView, 0, mReceiver);
		    	mView.resetCommitText();
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
