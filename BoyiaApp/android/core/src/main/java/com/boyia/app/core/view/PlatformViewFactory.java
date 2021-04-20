package com.boyia.app.core.view;

import android.content.Context;

interface PlatformViewFactory {
    PlatformView createView(Context context, String viewId);
}
