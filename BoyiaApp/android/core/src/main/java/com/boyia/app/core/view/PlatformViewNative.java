package com.boyia.app.core.view;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;

// 提供给c++进行调用
public class PlatformViewNative {
    private static final String TAG = "PlatformViewNative";

    // 创建NativeView
    public static void createPlatformView(
            String viewId,
            String viewType,
            int logicalWidth,
            int logicalHeight,
            int textureId) {
        BoyiaLog.i(TAG, "createPlatformView logicalWidth=" + logicalWidth + " logicalHeight=" + logicalHeight);
        BaseApplication.getInstance().getAppHandler().post(() -> {
            PlatformViewManager manager = PlatformViewManager.getInstance();
            PlatformViewController.PlatformViewCreationRequest request =
                    new PlatformViewController.PlatformViewCreationRequest(
                            viewId, viewType, logicalWidth, logicalHeight, textureId
                    );

            PlatformViewController controller =
                    new PlatformViewController(manager.getContext());
            controller.createPlatformView(request);
            manager.addPlatformView(viewId, controller);
        });

    }

    public static float[] updateTexture(String viewId) {
        PlatformViewManager manager = PlatformViewManager.getInstance();
        PlatformViewController controller = manager.getPlatformView(viewId);
        if (controller != null) {
            BoyiaLog.i(TAG, "controller is not null, id=" + viewId);
            return controller.updateTexture();
        } else {
            BoyiaLog.i(TAG, "controller is null, id=" + viewId);
        }

        return null;
    }
}
