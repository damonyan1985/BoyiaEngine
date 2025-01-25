package com.boyia.app.core.texture;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicLong;

public class BoyiaTextureManager {
    private final Map<Long, BoyiaTexture> mTextureRegistry = new ConcurrentHashMap<>();
    private final AtomicLong nextTextureId = new AtomicLong(1L);

    public static class BoyiaTextureManagerHolder {
        private static final BoyiaTextureManager INSTANCE = new BoyiaTextureManager();
    }

    public static BoyiaTextureManager getInstance() {
        return BoyiaTextureManagerHolder.INSTANCE;
    }

    public BoyiaTexture createTexture() {
        return registerTexture(BoyiaTexture.createTexture(nextTextureId.getAndIncrement()));
    }

    /**
     * 注册应用层外接纹理
     * @param texture
     */
    public BoyiaTexture registerTexture(BoyiaTexture texture) {
        mTextureRegistry.put(texture.getTextureId(), texture);
        return texture;
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
