package com.boyia.app.platform;

import android.content.Context;
import android.content.ContextWrapper;

public class PresentationContext extends ContextWrapper {
    public PresentationContext(Context context) {
        super(context);
    }
}
