package com.boyia.app.core.view;

import android.app.Presentation;
import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.view.Display;
import android.view.WindowManager;
import android.widget.FrameLayout;

/**
 * 参考flutter中platformview的实现
 * 一个viewId对应一个PlatformPresentation
 */
public class PlatformPresentation extends Presentation {
    private FrameLayout mRootView;
    private PlatformViewFactory mViewFactory;
    private String mViewId;
    private PlatformView mView;

    public PlatformPresentation(Context context,
                                String viewId,
                                PlatformViewFactory viewFactory,
                                Display display) {
        super(context, display);
        getWindow().setFlags(
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
                WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE);

        getWindow().setType(WindowManager.LayoutParams.TYPE_PRIVATE_PRESENTATION);

        mViewFactory = viewFactory;
        mViewId = viewId;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // This makes sure we preserve alpha for the VD's content.
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));

        mRootView = new FrameLayout(getContext());
        mView = mViewFactory.createView(getContext(),  mViewId);
        if (mView != null && mView.getView() != null) {
            mRootView.addView(mView.getView());
        }
        setContentView(mRootView);
    }

    public PlatformView getPlatformView() {
        return mView;
    }
}
