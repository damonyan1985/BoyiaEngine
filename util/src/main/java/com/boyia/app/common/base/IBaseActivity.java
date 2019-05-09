package com.boyia.app.common.base;

import android.view.View;
import android.view.ViewGroup.LayoutParams;

public interface IBaseActivity {
    void addView(View view, LayoutParams lp);
    BaseActivity getActivity();
    void removeView(View view);
}
