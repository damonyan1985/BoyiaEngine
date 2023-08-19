package com.boyia.app.core.device.camera;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Size;
import android.view.Surface;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.texture.BoyiaTexture;

import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.concurrent.Semaphore;

public class BoyiaCamera {
    private static final String TAG = "BoyiaCamera";
    private static final String CAMERA_ID = "0";
    // 在显示上实时绘制图像，类似视频
    private BoyiaTexture mTexture;
    // 捕捉某一时刻的图像
    private ImageReader mImageReader;
    private OnPreviewListener mOnPreviewListener;
    private boolean mIsNeedCapture;

    // 预览大小
    private Size mPreviewSize;

    // 相机回调线程
    private HandlerThread mCameraThread;
    private Handler mCameraHandler;

    // 相机设备
    private CameraDevice mCameraDevice;
    private CameraCaptureSession mCaptureSession;
    private CaptureRequest.Builder mPreviewBuilder;
    private String mCameraId;

    // 相机视频对应的纹理ID
    private int mTextureId;
    private Semaphore mLock = new Semaphore(1);

    private Surface mSurface;

    private CameraCaptureSession.StateCallback mCaptureSessionStateCallback = new CameraCaptureSession.StateCallback() {
        @Override
        public void onConfigured(CameraCaptureSession session) {
            if (mCameraDevice == null) {
                return;
            }

            mCaptureSession = session;
            try {
                mPreviewBuilder.set(CaptureRequest.CONTROL_AF_MODE,
                        CaptureRequest.CONTROL_AF_MODE_CONTINUOUS_PICTURE);

                mCaptureSession.setRepeatingRequest(
                        mPreviewBuilder.build(),
                        mCaptureCallback,
                        mCameraHandler
                );
            } catch (CameraAccessException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onConfigureFailed(CameraCaptureSession session) {
            mCaptureSession = session;
        }
    };

    private CameraDevice.StateCallback mDeviceStateCallback = new CameraDevice.StateCallback() {
        @Override
        public void onOpened(CameraDevice camera) {
            mCameraDevice = camera;
            createCameraPreview();
        }

        @Override
        public void onDisconnected(CameraDevice camera) {
            mCameraDevice.close();
            mCameraDevice = null;
        }

        @Override
        public void onError(CameraDevice camera, int error) {

        }
    };

    private CameraCaptureSession.CaptureCallback mCaptureCallback
            = new CameraCaptureSession.CaptureCallback() {

        @Override
        public void onCaptureProgressed(CameraCaptureSession session,
                                        CaptureRequest request,
                                        CaptureResult partialResult) {
        }

        @Override
        public void onCaptureCompleted(CameraCaptureSession session,
                                       CaptureRequest request,
                                       TotalCaptureResult result) {
        }

    };

    public BoyiaCamera(int textureId, boolean needCapture) {
        mTextureId = textureId;
        mIsNeedCapture = needCapture;
    }

    public BoyiaCamera(int textureId) {
        this(textureId, false);
    }

    boolean configCamera(CameraManager cameraManager, String cameraId) throws CameraAccessException {
        CameraCharacteristics characteristics = cameraManager.getCameraCharacteristics(cameraId);
        StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        if (map == null) {
            return false;
        }

        return true;
    }

    private void stopCameraThread() {
        mCameraThread.quitSafely();
        try {
            mCameraThread.join();
            mCameraThread = null;
            mCameraHandler = null;
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    // 初始化线程
    @SuppressLint("DefaultLocale")
    void initThread() {
        mCameraThread = new HandlerThread(String.format("boyia-camera-%d", mTextureId));
        mCameraHandler = new Handler(mCameraThread.getLooper());
    }

    void openCamera() {
        CameraManager cameraManager = (CameraManager) BaseApplication.getInstance().getSystemService(Context.CAMERA_SERVICE);
        try {
            if (configCamera(cameraManager, CAMERA_ID)) {
                return;
            }

            for (String id : cameraManager.getCameraIdList()) {
                if (configCamera(cameraManager, id)) {
                    return;
                }
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
            BoyiaLog.e(TAG, "creatCamera error", e);
        }

        initThread();
        if (BaseApplication.getInstance().checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            return;
        }

        setUpCameraOutputs(cameraManager);

        try {
            // 打开摄像头
            cameraManager.openCamera(mCameraId, mDeviceStateCallback, mCameraHandler);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void setUpCameraOutputs(CameraManager cameraManager) {
        try {
            for (String cameraId : cameraManager.getCameraIdList()) {
                CameraCharacteristics characteristics
                        = cameraManager.getCameraCharacteristics(cameraId);

                // We don't use a front facing camera in this sample.
                Integer facing = characteristics.get(CameraCharacteristics.LENS_FACING);
                if (facing != null && facing == CameraCharacteristics.LENS_FACING_BACK) {
                    continue;
                }
                StreamConfigurationMap map = characteristics.get(
                        CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                if (map == null) {
                    continue;
                }

                if (mIsNeedCapture) {
                    mImageReader = ImageReader.newInstance(300, 300, ImageFormat.YUV_420_888, 2);
                    mImageReader.setOnImageAvailableListener(mOnImageAvailableListener, mCameraHandler);
                }
                mCameraId = cameraId;
                return;
            }
        } catch (CameraAccessException e) {
            e.printStackTrace();
        } catch (NullPointerException e) {

        }
    }

    private void createCameraPreview() {
        try {
            mTexture = new BoyiaTexture(mTextureId);
            mSurface = new Surface(mTexture.getSurfaceTexture());
            mPreviewBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewBuilder.addTarget(mSurface);

            // 创建session
            mCameraDevice.createCaptureSession(
                    Arrays.asList(mSurface),
                    mCaptureSessionStateCallback,
                    mCameraHandler
            );
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
    }

    private void closeCamera() {
        if (null != mCaptureSession) {
            mCaptureSession.close();
            mCaptureSession = null;
        }

        if (null != mCameraDevice) {
            mCameraDevice.close();
            mCameraDevice = null;
        }

        if (null != mSurface) {
            mSurface.release();
            mSurface = null;
        }
    }

    private ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireNextImage();
            if (image == null) {
                return;
            }

            Image.Plane[] planes = image.getPlanes();
            int width = image.getWidth();
            int height = image.getHeight();

            byte[] yBytes = new byte[width * height];
            byte[] uBytes = new byte[width * height / 4];
            byte[] vBytes = new byte[width * height / 4];
            byte[] i420 = new byte[width * height * 3 / 2];



            for (int i = 0; i < planes.length; i++) {
                int dstIndex = 0;
                int uIndex = 0;
                int vIndex = 0;
                int pixelStride = planes[i].getPixelStride();
                int rowStride = planes[i].getRowStride();

                ByteBuffer buffer = planes[i].getBuffer();

                byte[] bytes = new byte[buffer.capacity()];

                buffer.get(bytes);
                int srcIndex = 0;
                if (i == 0) {
                    for (int j = 0; j < height; j++) {
                        System.arraycopy(bytes, srcIndex, yBytes, dstIndex, width);
                        srcIndex += rowStride;
                        dstIndex += width;
                    }
                } else if (i == 1) {
                    for (int j = 0; j < height / 2; j++) {
                        for (int k = 0; k < width / 2; k++) {
                            uBytes[dstIndex++] = bytes[srcIndex];
                            srcIndex += pixelStride;
                        }

                        if (pixelStride == 2) {
                            srcIndex += rowStride - width;
                        } else if (pixelStride == 1) {
                            srcIndex += rowStride - width / 2;
                        }
                    }
                } else if (i == 2) {
                    for (int j = 0; j < height / 2; j++) {
                        for (int k = 0; k < width / 2; k++) {
                            vBytes[dstIndex++] = bytes[srcIndex];
                            srcIndex += pixelStride;
                        }

                        if (pixelStride == 2) {
                            srcIndex += rowStride - width;
                        } else if (pixelStride == 1) {
                            srcIndex += rowStride - width / 2;
                        }
                    }
                }
                System.arraycopy(yBytes, 0, i420, 0, yBytes.length);
                System.arraycopy(uBytes, 0, i420, yBytes.length, uBytes.length);
                System.arraycopy(vBytes, 0, i420, yBytes.length + uBytes.length, vBytes.length);

                if (mOnPreviewListener != null) {
                    mOnPreviewListener.onPreviewFrame(i420, i420.length);
                }


            }
            image.close();
        }
    };

    public void setOnPreviewListener(OnPreviewListener onPreviewListener) {
        this.mOnPreviewListener = onPreviewListener;
    }

    public interface OnPreviewListener {
        void onPreviewFrame(byte[] data, int len);
    }

}
