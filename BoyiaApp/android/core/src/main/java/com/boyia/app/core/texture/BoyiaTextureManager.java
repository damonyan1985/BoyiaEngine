package com.boyia.app.core.texture;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class BoyiaTextureManager {
    private Map<Long, BoyiaTexture> mTextureRegistry = new ConcurrentHashMap<>();

    public static class BoyiaTextureManagerHolder {
        private static final BoyiaTextureManager INSTANCE = new BoyiaTextureManager();
    }

    public static BoyiaTextureManager getInstance() {
        return BoyiaTextureManagerHolder.INSTANCE;
    }

    /**
     * 注册应用层外接纹理
     * @param texture
     */
    public void registerTexture(BoyiaTexture texture) {
        mTextureRegistry.put(texture.getTextureId(), texture);
    }

    public void attach(long textureId, int textName) {
        BoyiaTexture texture = mTextureRegistry.get(textureId);
        if (texture != null) {
            texture.attach(textName);
        }
    }

    public void detach(long textureId) {
        BoyiaTexture texture = mTextureRegistry.get(textureId);
        if (texture != null) {
            texture.detach();
        }
    }

    public float[] update(long textureId) {
        BoyiaTexture texture = mTextureRegistry.get(textureId);
        if (texture != null) {
            return texture.updateTexture();
        }

        return null;
    }
}
