package com.boyia.app.common.base;

import android.content.res.Resources;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnKeyListener;
import android.view.View.OnTouchListener;
import android.view.ViewGroup.LayoutParams;

public class BaseWindow {
	private static final String TAG = BaseWindow.class.getSimpleName();
	protected IBaseActivity mActivity = null;
	protected View mView = null;

	public BaseWindow(IBaseActivity activity) {
		mActivity = activity;
	}

	public void show() {
		if (mView == null) {
		    onCreate();		
		    addViewToWindow();
		} else {
			mView.setVisibility(View.VISIBLE);
		}
		
		mView.setFocusable(true);
		mView.setFocusableInTouchMode(true);
		mView.requestFocus();
	}
	
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		return false;
	}
	
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		return false;
	}

	public void hide() {
		if (mView != null) {
			mView.setVisibility(View.GONE);
		}
	}

	protected void onCreate() {
	}

	public void onDestory() {
		if (mView != null) {
			detachView();
			mView = null;
		}
	}

	public BaseActivity getActivity() {
		return mActivity.getActivity();
	}
	
	public void setContentView(View view) {
		mView = view;
	}

	public void setContentView(int layout) {
        // Create view width xml
		mView = View.inflate(getActivity(), layout, null);
	}

	private void addViewToWindow() {
		if (mView != null && mView.getParent() == null) {
			LayoutParams lp = new LayoutParams(LayoutParams.MATCH_PARENT,
					LayoutParams.MATCH_PARENT);
			mActivity.addView(mView, lp);
		} else if (mView != null && mView.getParent() != null) {
			mView.setVisibility(View.VISIBLE);
		}
		
		if (mView != null) {
			// Set Key Event Listener
			mView.setOnKeyListener(new OnKeyListener() {
				@Override
				public boolean onKey(View v, int keyCode, KeyEvent event) {
					if (event.getAction() == KeyEvent.ACTION_DOWN) {					
						return BaseWindow.this.onKeyDown(keyCode, event);
					} else if (event.getAction() == KeyEvent.ACTION_UP) {
						return BaseWindow.this.onKeyUp(keyCode, event);
					}
					
					return false;
				}
			});
		
			// Set TouchEvent Listener
			mView.setOnTouchListener(new OnTouchListener() {
				@Override
				public boolean onTouch(View v, MotionEvent event) {
					return BaseWindow.this.onTouchEvent(event);
				}
			});
		}
	}
	
	public boolean onTouchEvent(MotionEvent event) {
		return false;
	}

	public boolean isVisible() {
		return mView != null && mView.getVisibility() == View.VISIBLE;
	}

	public View findViewById(int id) {
		if (mView == null) {
			return null;
		} else {
			return mView.findViewById(id);
		}
	}

	public Resources getResources() {
		if (mActivity == null) {
			return null;
		} else {
			return mView.getResources();
		}
	}
	
	public void detachView() {
		if (mActivity != null) {
			mActivity.removeView(mView);
			mView = null;
		}
	}
	
    public void requestFocus() {
        mView.requestFocus();
    }
    
	public void lostFocus() {
		mView.setFocusable(false);
	}
}