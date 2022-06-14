package com.boyia.app.core.texture;

import android.graphics.SurfaceTexture;
import android.view.Surface;

public class BoyiaTexture implements SurfaceTexture.OnFrameAvailableListener {
    private SurfaceTexture mTexture;
    private Surface mSurface;
    private float[] mSTMatrix = new float[16];
    private boolean mUpdateSurface = false;
    private static long mLastPlayTime = 0;
    private TextureUpdateNotifier mNotifier = null;

    public BoyiaTexture(int textureId) {
        mTexture = new SurfaceTexture(textureId);
        mTexture.setOnFrameAvailableListener(this);
        mSurface = new Surface(mTexture);
    }

    public Surface getSurface() {
        return mSurface;
    }

    public SurfaceTexture getSurfaceTexture() {
        return mTexture;
    }

    public void setTextureUpdateNotifier(TextureUpdateNotifier notifier) {
        mNotifier = notifier;
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        // Notify Boyia Framework To Renderer
        synchronized (this) {
            mUpdateSurface = true;
        }

        if (0 == mLastPlayTime) {
            mLastPlayTime = System.currentTimeMillis();
        } else {
            // 小于16毫秒再来一次，就放弃绘制
            if (System.currentTimeMillis() - mLastPlayTime < 16) {
                return;
            }
        }

        if (mNotifier != null) {
            mNotifier.onTextureUpdate();
        }
    }

    public float[] updateTexture() {
        try {
            mTexture.updateTexImage();
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        try {
            mTexture.getTransformMatrix(mSTMatrix);
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        synchronized (this) {
            mUpdateSurface = false;
        }

        return mSTMatrix;
    }

    public boolean canDraw() {
        return mUpdateSurface;
    }

    public interface TextureUpdateNotifier {
        void onTextureUpdate();
    }
}
