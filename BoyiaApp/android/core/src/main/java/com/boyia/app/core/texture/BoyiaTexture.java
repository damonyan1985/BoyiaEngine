package com.boyia.app.core.texture;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.BoyiaBridge;
import com.boyia.app.core.BoyiaCoreJNI;

import java.util.concurrent.atomic.AtomicLong;

public class BoyiaTexture implements SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "BoyiaTexture";
    private static final AtomicLong nextTextureId = new AtomicLong(1L);
    private SurfaceTexture mTexture;

    private long mTextureId;
    private Surface mSurface;
    private float[] mSTMatrix = new float[16];
    private boolean mUpdateSurface = false;
    private static long mLastPlayTime = 0;
    private TextureUpdateNotifier mNotifier = null;
    // 是否attach至GLContext
    private boolean mIsAttached = false;

    public static BoyiaTexture createTexture() {
        BoyiaTexture texture = new BoyiaTexture(0);
        texture.mTextureId = nextTextureId.getAndIncrement();
        BoyiaLog.d(TAG, "createTexture tid=" +texture.mTextureId);
        BoyiaTextureManager.getInstance().registerTexture(texture);
        return texture;
    }

    public BoyiaTexture(int textureId) {
        mTexture = new SurfaceTexture(textureId);
        mTexture.setOnFrameAvailableListener(this);
        mSurface = new Surface(mTexture);
        mTexture.detachFromGLContext();
    }

    public long getTextureId() {
        return mTextureId;
    }

    /**
     * use opengl texture id to attach to gl context
     * @param texName
     */
    public void attach(int texName) {
        synchronized (this) {
            if (mIsAttached) {
                mTexture.detachFromGLContext();
            }

            BoyiaLog.d(TAG, "BoyiaTexture attached texName = " + texName);
            mTexture.attachToGLContext(texName);
            mIsAttached = true;
        }
    }

    /**
     * detach from gl context
     */
    public void detach() {
        synchronized (this) {
            mTexture.detachFromGLContext();
        }
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
        BoyiaLog.d(TAG, "onFrameAvailable call");
        if (mNotifier != null) {
            mNotifier.onTextureUpdate();
        }
    }

    // 必须调用updateTexImage, onFrameAvailable才会继续被调用
    public float[] updateTexture() {
        BoyiaLog.d(TAG, "updateTexture call");
        synchronized (this) {
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
