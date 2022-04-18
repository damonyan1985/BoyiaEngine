package com.boyia.app.common.plugin;

import android.content.res.AssetManager;

/**
 * 插件AssetManager
 */
public class BoyiaAssetHooker {
    private static final String CLASS_NAME = "android.content.res.AssetManager";
    private static final String METHOD_ADD_ASSSET_PATH = "addAssetPath";
    private static final String METHOD_ENSURE_STRING_BLOCKS = "ensureStringBlocks";

    private Object sInstance;

    public BoyiaAssetHooker() {
        try {
            sInstance = AssetManager.class.newInstance();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        }
    }

    /**
     * 加载apk对应的资源
     * @param path Apk路径 此处为插件APK的路径
     */
    public void addAssetPath(String path) {
        BoyiaMethodInvoker.invokeMethod(sInstance, CLASS_NAME, METHOD_ADD_ASSSET_PATH, new Class[]{String.class}, new Object[]{path});
    }

    /**
     * 初始化其内部参数
     * @return
     */
    public Object[] ensureStringBlocks() {
        return (Object[])BoyiaMethodInvoker.invokeMethod(sInstance,
                CLASS_NAME, METHOD_ENSURE_STRING_BLOCKS, null, null);
    }
}
