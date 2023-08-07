package com.boyia.app.core;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.texture.BoyiaTexture;

import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.MediaPlayer.OnBufferingUpdateListener;
import android.media.MediaPlayer.OnCompletionListener;
import android.media.MediaPlayer.OnErrorListener;
import android.media.MediaPlayer.OnInfoListener;
import android.media.MediaPlayer.OnPreparedListener;
import android.media.MediaPlayer.OnSeekCompleteListener;
import android.media.MediaPlayer.OnVideoSizeChangedListener;
import android.net.Uri;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

public class BoyiaPlayer implements OnBufferingUpdateListener,
        OnCompletionListener, OnPreparedListener, OnVideoSizeChangedListener,
        OnErrorListener, OnSeekCompleteListener, OnInfoListener,
        BoyiaTexture.TextureUpdateNotifier {
    private static final String TAG = "BoyiaPlayer";
    private MediaPlayer mPlayer = null;
    private Uri mPlayerUri = null;
    private long mNativePtr = 0;
    private HandlerThread mThread;
    private Handler mHandler;
    private BoyiaTexture mTexture;

    private long mPlayerId = 0;

    private static class PlayerHandler extends Handler {
        public static final int GL_VIDEO_START = 0;
        public static final int GL_VIDEO_PAUSE = 1;
        public static final int GL_VIDEO_STOP = 2;
        public static final int GL_VIDEO_SEEK = 3;

        public PlayerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            BoyiaPlayer player = (BoyiaPlayer) msg.obj;
            switch (msg.what) {
                case GL_VIDEO_START:
                    player.initPlayer();
                    player.prepare();
                    break;
                case GL_VIDEO_PAUSE:
                    player.mPlayer.pause();
                    break;
                case GL_VIDEO_STOP:
                    player.mPlayer.release();
                    break;
                case GL_VIDEO_SEEK:
                    player.mPlayer.seekTo((int) msg.arg1);
                    break;
            }
        }
    }

    public BoyiaPlayer() {
        mTexture = BoyiaTexture.createTexture();
        mTexture.setTextureUpdateNotifier(this);

        mPlayerId = mTexture.getTextureId();
        mThread = new HandlerThread(String.valueOf(mPlayerId));
        mThread.start();
        mHandler = new PlayerHandler(mThread.getLooper());
    }

    // Call by native mediaplayer
    public void setNativePtr(long nativePtr) {
        mNativePtr = nativePtr;
    }

    /**
     * Call by native player start
     * @param url
     */
    public void start(String url) {
        BoyiaLog.d(TAG, "BoyiaPlayer start url = " + url);
        mPlayerUri = Uri.parse(url);

        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_START;
        msg.obj = this;
        mHandler.sendMessage(msg);
    }

    /**
     * Call by native player pause
     */
    public void pause() {
        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_PAUSE;
        msg.obj = this;
        mHandler.sendMessage(msg);
    }

    /**
     * Call by native player stop
     */
    public void stop() {
        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_PAUSE;
        msg.obj = this;
        mHandler.sendMessage(msg);
    }

    /**
     * Call by native player seek
     * @param progress
     */
    public void seek(int progress) {
        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_PAUSE;
        msg.obj = this;
        msg.arg1 = progress;
        mHandler.sendMessage(msg);
    }

    private void initPlayer() {
        if (mPlayer == null) {
            BoyiaLog.d(TAG, "BoyiaPlayer initPlayer");

            mPlayer = new MediaPlayer();
            mPlayer.setSurface(mTexture.getSurface());
            mTexture.getSurface().release();
        }
    }

    /**
     * Call by native player getPlayerId
     */
    public long getPlayerId() {
        return mPlayerId;
    }

    public void prepare() {
        initListener();

        try {
            mPlayer.setDataSource(BaseApplication.getInstance(),
                    mPlayerUri);
            mPlayer.setScreenOnWhilePlaying(true);
            mPlayer.prepareAsync();
        } catch (Throwable e) {
            BoyiaLog.e(TAG, "BoyiaPlayer prepare error, context = " + BaseApplication.getInstance(), e);
        }
    }

    private void initListener() {
        mPlayer.setOnBufferingUpdateListener(this);
        mPlayer.setOnCompletionListener(this);
        mPlayer.setOnPreparedListener(this);
        mPlayer.setOnVideoSizeChangedListener(this);
        mPlayer.setAudioStreamType(AudioManager.STREAM_MUSIC);
        mPlayer.setOnErrorListener(this);
        mPlayer.setOnSeekCompleteListener(this);
        mPlayer.setOnInfoListener(this);
    }

    @Override
    public boolean onInfo(MediaPlayer mp, int what, int extra) {
        return false;
    }

    @Override
    public void onSeekComplete(MediaPlayer mp) {

    }

    @Override
    public boolean onError(MediaPlayer mp, int what, int extra) {
        return false;
    }

    @Override
    public void onVideoSizeChanged(MediaPlayer mp, int width, int height) {

    }

    @Override
    public void onPrepared(MediaPlayer mp) {
        BoyiaLog.d(TAG, "onPrepared");
        mp.start();
    }

    @Override
    public void onCompletion(MediaPlayer mp) {

    }

    @Override
    public void onBufferingUpdate(MediaPlayer mp, int percent) {
    }

    // Call by native mediaplayer
    public float[] updateTexture() {
        BoyiaLog.d("libboyia",
                "BoyiaPlayer updateTexImage texId=" + mTexture.getTextureId() + " threadId=" + Thread.currentThread().getId());
        return mTexture.updateTexture();
    }

    // Call by native mediaplayer
    public boolean canDraw() {
        if (mTexture == null) {
            return false;
        }

        return mTexture.canDraw();
    }

    @Override
    public void onTextureUpdate() {
        BoyiaLog.d("libboyia", "onFrameAvailable id=" + mTexture.getTextureId() + " threadId=" + Thread.currentThread().getId());
        BoyiaCoreJNI.nativeVideoTextureUpdate(mNativePtr);
    }
}
