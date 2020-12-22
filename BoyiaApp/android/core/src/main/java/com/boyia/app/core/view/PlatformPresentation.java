package com.boyia.app.core.view;

import android.app.Presentation;
import android.content.Context;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.view.Display;
import android.view.WindowManager;
import android.widget.FrameLayout;

/**
 * 参考flutter中platformview的实现
 */
public class PlatformPresentation extends Presentation {
    private FrameLayout mRootView;
    private PlatformView mPlatformView;

    public PlatformPresentation(Context context,
                                PlatformView view,
                                Display display) {
        super(context, display);
        getWindow().setFlags(
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);

        getWindow().setType(WindowManager.LayoutParams.TYPE_PRIVATE_PRESENTATION);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // This makes sure we preserve alpha for the VD's content.
        getWindow().setBackgroundDrawable(new ColorDrawable(android.graphics.Color.TRANSPARENT));

        mRootView = new FrameLayout(getContext());
        mRootView.addView(mPlatformView.getView());
    }
}
