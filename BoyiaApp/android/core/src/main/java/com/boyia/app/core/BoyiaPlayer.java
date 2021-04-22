package com.boyia.app.core;

import java.io.IOException;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.texture.BoyiaTexture;

import android.graphics.SurfaceTexture;
import android.graphics.SurfaceTexture.OnFrameAvailableListener;
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
import android.view.Surface;

public class BoyiaPlayer implements OnBufferingUpdateListener,
        OnCompletionListener, OnPreparedListener, OnVideoSizeChangedListener,
        OnErrorListener, OnSeekCompleteListener, OnInfoListener,
        BoyiaTexture.TextureUpdateNotifier {
    private MediaPlayer mPlayer = null;
    private Uri mPlayerUri = null;
    private long mNativePtr = 0;
    private HandlerThread mThread;
    private Handler mHandler;
    private int mTextureID = 0;
    private BoyiaTexture mTexture;

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
        mThread = new HandlerThread(this.toString());
        mThread.start();
        mHandler = new PlayerHandler(mThread.getLooper());
    }

    // Call by native mediaplayer
    public void setNativePtr(long nativePtr) {
        mNativePtr = nativePtr;
    }

    public void start(String url) {
        mPlayerUri = Uri.parse(url);

        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_START;
        msg.obj = this;
        mHandler.sendMessage(msg);
    }

    public void pause() {
        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_PAUSE;
        msg.obj = this;
        mHandler.sendMessage(msg);
    }

    public void stop() {
        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_PAUSE;
        msg.obj = this;
        mHandler.sendMessage(msg);
    }

    public void seek(int progress) {
        Message msg = Message.obtain();
        msg.what = PlayerHandler.GL_VIDEO_PAUSE;
        msg.obj = this;
        msg.arg1 = progress;
        mHandler.sendMessage(msg);
    }

    private void initPlayer() {
        if (mPlayer == null) {
            mTexture = new BoyiaTexture(mTextureID);
            mTexture.setTextureUpdateNotifier(this);

            mPlayer = new MediaPlayer();
            mPlayer.setSurface(mTexture.getSurface());
            mTexture.getSurface().release();
        }
    }

    // Call by native mediaplayer
    public void setTextureId(int id) {
        mTextureID = id;
    }

    public void prepare() {
        initListener();

        try {
            mPlayer.setDataSource(BaseApplication.getInstance(),
                    mPlayerUri);
            mPlayer.setScreenOnWhilePlaying(true);
            mPlayer.prepareAsync();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        } catch (SecurityException e) {
            e.printStackTrace();
        } catch (IllegalStateException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
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
        BoyiaLog.d("libboyia", "onPrepared");
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
                "BoyiaPlayer updateTexImage texId=" + mTextureID + " threadId=" + Thread.currentThread().getId());
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
        BoyiaLog.d("libboyia", "onFrameAvailable id=" + this.mTextureID + " threadId=" + Thread.currentThread().getId());
        BoyiaCoreJNI.nativeVideoTextureUpdate(mNativePtr);
    }
}
