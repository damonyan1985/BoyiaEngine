package com.boyia.app.core.view;

import android.app.Presentation;
import android.content.Context;
import android.os.Bundle;
import android.view.Display;

/**
 * 参考flutter中platformview的实现
 */
public class PlatformPresentation extends Presentation {
    public PlatformPresentation(Context context,
                                Display display) {
        super(context, display);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
}
