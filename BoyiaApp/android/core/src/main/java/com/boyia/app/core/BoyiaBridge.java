package com.boyia.app.core;

import android.widget.Toast;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.api.ApiHandler;
import com.boyia.app.core.api.ApiImplementation;
import com.boyia.app.core.texture.BoyiaTextureManager;
import com.boyia.app.loader.mue.MainScheduler;

import java.lang.ref.WeakReference;

/**
 * Native Core与框架层桥接类
 */
public class BoyiaBridge {
    private static final String TAG = "BoyiaBridge";
    private static WeakReference<ApiImplementation> sApiImplementation;

    /**
     * 获取文本长度
     * @param text
     * @return
     */
    public static int getTextSize(String text) {
        return text.length();
    }

    /**
     * boyia应用目录
     * @return
     */
    public static String getAppRoot() {
        return BoyiaFileUtil.getAppRoot();
    }

    // dpi为每英寸所占的像素
    public static float getDisplayDensity() {
        // 每英寸占用的像素除以160
        float density = BaseApplication.getInstance().
                getResources().getDisplayMetrics().density;
        BoyiaLog.i(TAG, "BoyiaBridge.getDisplayDensity() = " + density);
        return density;
    }

    public static void showToast(final String info) {
        BoyiaLog.d(TAG, "toast=" + info);
        MainScheduler.mainScheduler().sendJob(() -> {
            Toast.makeText(BaseApplication.getInstance(), info,
                    Toast.LENGTH_SHORT).show();
        });
    }

    /**
     * 设置IPC发送器，为boyia app提供宿主api功能
     */
    public static void setIPCSender(ApiImplementation implementation) {
        sApiImplementation = new WeakReference<>(implementation);
    }

    /**
     * Boyia native engine反射调用系统api
     * @param json
     * @param nativeCB
     */
    public static void callApi(String json, long nativeCB) {
        if (sApiImplementation != null && sApiImplementation.get() != null) {
            sApiImplementation.get().handleApi(json, new ApiHandler.ApiHandlerCallback() {
                long nativePtr() {
                    return nativeCB;
                }

                /**
                 * 利用nativeCB回传给引擎
                 * @param json
                 */
                @Override
                public void callback(String json) {
                }
            });
        }
    }

    /**
     * Call by engine native code
     */
    public static void attachTexture(long textureId, int textName) {
        BoyiaLog.d(TAG, "textureId = " + textureId + "; and textName = " + textName);
        BoyiaTextureManager.getInstance().attach(textureId, textName);
    }

    /**
     * Call by engine native code
     */
    public static void detachTexture(long textureId) {
        BoyiaTextureManager.getInstance().detach(textureId);
    }

    /**
     * Call by engine native code
     */
    public static float[] updateTexture(long textureId) {
        return BoyiaTextureManager.getInstance().update(textureId);
    }
}