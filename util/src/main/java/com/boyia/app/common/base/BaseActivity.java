package com.boyia.app.common.base;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.PowerManager;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewGroup.LayoutParams;
import android.view.WindowManager;

public class BaseActivity extends Activity implements IBaseActivity {
	private static final String TAG = BaseActivity.class.getSimpleName();
    private ViewGroup mMainView;
    private Handler mUIHandler = null;
    private PowerManager.WakeLock mWakeLock = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mUIHandler = new Handler();
    }
    
	@Override
	protected void onResume() {
		super.onResume();
		acquireWakeLock();
	}
	
	@Override
	public void setContentView(View view, LayoutParams layout) {
		mMainView = (ViewGroup) view;
		super.setContentView(view, layout);
	}

    @Override
    public void setContentView(int resId) {
        mMainView = (ViewGroup) View.inflate(this, resId,
                null);
        super.setContentView(mMainView);
    }
    
    @Override
    public void setContentView(View view) {
        mMainView = (ViewGroup) view;
        super.setContentView(view);
    }

    protected ViewGroup getMainView() {
        return mMainView;
    }
    
    public Handler getUIHandler() {
    	return mUIHandler;
    }
    
    @SuppressWarnings("deprecation")
	private void acquireWakeLock() {
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		if (mWakeLock == null) {
			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
			mWakeLock = pm.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ON_AFTER_RELEASE, TAG);
			mWakeLock.acquire();
		} else if (null != mWakeLock && (!mWakeLock.isHeld())) {
			mWakeLock.acquire();
		}
	}
    
    @Override
    public void onDestroy() {
    	super.onDestroy();
    	if (mWakeLock != null) {
    	    mWakeLock.release();
    	}
    }
    
    @Override
    public boolean onTouchEvent(MotionEvent event) {
    	return super.onTouchEvent(event);
    }

	@Override
	public void addView(View view, LayoutParams lp) {
		getMainView().addView(view, lp);
	}

	@Override
	public BaseActivity getActivity() {
		return this;
	}

	@Override
	public void removeView(View view) {
		mMainView.removeView(view);
	}
}
