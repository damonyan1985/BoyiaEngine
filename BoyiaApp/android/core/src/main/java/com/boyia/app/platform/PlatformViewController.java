package com.boyia.app.platform;

import android.app.Presentation;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.hardware.display.DisplayManager;
import android.hardware.display.VirtualDisplay;
import android.view.Surface;

/**
 * 同层渲染，本地view控制类
 */
public class PlatformViewController {
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
}
