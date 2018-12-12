package com.boyia.app.core;

import java.io.IOException;

import com.boyia.app.BoyiaApplication;
import com.boyia.app.utils.BoyiaLog;

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
		OnFrameAvailableListener {
	private MediaPlayer mPlayer = null;
	private SurfaceTexture mTexture = null;
	//private int[] mTextureNames = null;
	private Uri mPlayerUri = null;
	private long mNativePtr = 0;
	private float[] mSTMatrix = new float[16];
	private boolean mUpdateSurface = false;
	private static HandlerThread mThread = null;
	private static Handler mHandler = null;
	private int mTextureID = 0;
	private static long mLastPlayTime = 0;
	
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
    			player.mPlayer.seekTo((int)msg.arg1);
    			break;
    		}
    		
    	}
    }

    // 多个播放器共用一个播放线程
	public BoyiaPlayer() {
		if (mThread == null) {
			mThread = new HandlerThread("glvideo");
			mThread.start();
			mHandler = new PlayerHandler(mThread.getLooper());
		}
	}

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
			mTexture = new SurfaceTexture(mTextureID);
			mTexture.setOnFrameAvailableListener(this);
			Surface surface = new Surface(mTexture);

			mPlayer = new MediaPlayer();
			mPlayer.setSurface(surface);
			surface.release();
		}
	}
	
	public void setTextureId(int id) {
		mTextureID = id;
	}

	public void prepare() {
		initListener();

		try {
			mPlayer.setDataSource(BoyiaApplication.getCurrenContext(),
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

	public float[] updateTexture() {
		try {
			mTexture.updateTexImage();
		} catch (Exception ex) {
			ex.printStackTrace();
		}
		
		try {
			mTexture.getTransformMatrix(mSTMatrix);
			BoyiaLog.d("libboyia", "MiniMediaPlayer updateTexImage texId="
					+ mTextureID + " threadId="+Thread.currentThread().getId());
		} catch(Exception ex) {
		    ex.printStackTrace();	
		}
		
		synchronized(this) {
		    mUpdateSurface = false;
		}

		return mSTMatrix;
	}
	
	public boolean canDraw() {
		return mUpdateSurface;
	}

	@Override
	public void onFrameAvailable(SurfaceTexture texture) {
		//texture.updateTexImage();
		BoyiaLog.d("libboyia",
				"onFrameAvailable id=" + this.mTextureID + " threadId="+Thread.currentThread().getId());
		// Notify MiniYan Framework To Renderer
		synchronized(this) {
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
		
		BoyiaUIView.nativeVideoTextureUpdate(mNativePtr);
	}
}
