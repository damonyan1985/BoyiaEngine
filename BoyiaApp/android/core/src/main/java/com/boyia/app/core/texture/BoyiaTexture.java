package com.boyia.app.core.texture;

import android.graphics.SurfaceTexture;
import android.view.Surface;

import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.BoyiaBridge;
import com.boyia.app.core.BoyiaCoreJNI;

import java.util.concurrent.atomic.AtomicLong;

public class BoyiaTexture implements SurfaceTexture.OnFrameAvailableListener {
    private static final String TAG = "BoyiaTexture";

    private SurfaceTexture mTexture;
    /**
     * 自增id，非真实的纹理ID
     */
    private long mTextureId;
    /**
     * 获取图形的变换举证
     */
    private float[] mSTMatrix = new float[16];
    /**
     * 变化通知
     */
    private TextureUpdateNotifier mNotifier = null;
    /**
     * 是否attach至GLContext
     */
    private boolean mIsAttached = false;

    public static BoyiaTexture createTexture(long id) {
        BoyiaTexture texture = new BoyiaTexture(0);
        texture.mTexture.detachFromGLContext();
        texture.mTextureId = id;
        BoyiaLog.d(TAG, "createTexture tid=" +texture.mTextureId);
        return texture;
    }

    public BoyiaTexture(int textureId) {
        mTexture = new SurfaceTexture(textureId);
        mTexture.setOnFrameAvailableListener(this);
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
            mIsAttached = false;
        }
    }

    public SurfaceTexture getSurfaceTexture() {
        return mTexture;
    }

    public void setTextureUpdateNotifier(TextureUpdateNotifier notifier) {
        mNotifier = notifier;
    }

    /**
     * 下一帧回调
     * @param surfaceTexture
     */
    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        BoyiaLog.d(TAG, "onFrameAvailable call");
        if (mNotifier != null) {
            mNotifier.onTextureUpdate();
        }
    }

    /**
     * 获取当前帧
     * @return
     */
    public float[] updateTexture() {
        BoyiaLog.d(TAG, "updateTexture call");
        synchronized (this) {
            try {
                // 必须调用updateTexImage, onFrameAvailable才会继续被调用
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

    public interface TextureUpdateNotifier {
        void onTextureUpdate();
    }
}
