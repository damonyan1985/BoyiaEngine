package com.boyia.app.core.view;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.view.Surface;

/**
 * 同层渲染，本地view控制类
 */
public class PlatformViewController {
    private Context mContext;
    // 创建VirtualDisplay
    public VirtualDisplay createVirtualDisplay(Context context, int texId, int width, int height) {
        SurfaceTexture texture = new SurfaceTexture(texId);
        Surface surface = new Surface(texture);
        DisplayManager displayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
        int densityDpi = context.getResources().getDisplayMetrics().densityDpi;
        return displayManager.createVirtualDisplay(
                "boyia-vd",
                width,
                height,
                densityDpi,
                surface,
                0);
    }

    private int toPhysicalPixels(double logicalPixels) {
        float density = mContext.getResources().getDisplayMetrics().density;
        return (int) Math.round(logicalPixels * density);
    }
}
