package com.boyia.app.platform;

import android.app.Presentation;
import android.content.Context;
import android.view.Display;

public class PlatformPresentation extends Presentation {
    public PlatformPresentation(Context context,
                                Display display) {
        super(context, display);
    }
}
