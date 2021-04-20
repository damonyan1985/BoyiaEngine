package com.boyia.app.core.view;

import com.boyia.app.common.BaseApplication;

// 提供给c++进行调用
public class PlatformViewNative {

    // 创建NativeView
    public static void createPlatformView(
            String viewId,
            String viewType,
            int logicalWidth,
            int logicalHeight,
            int textureId) {
        BaseApplication.getInstance().getAppHandler().post(() -> {
            PlatformViewManager manager = PlatformViewManager.getInstance();
            PlatformViewController.PlatformViewCreationRequest request =
                    new PlatformViewController.PlatformViewCreationRequest(
                            viewId, viewType, logicalWidth, logicalHeight, textureId
                    );

            PlatformViewController controller = new PlatformViewController(manager.getContext());
            controller.createPlatformView(request);
            manager.addPlatformView(viewId, controller);
        });

    }
}
