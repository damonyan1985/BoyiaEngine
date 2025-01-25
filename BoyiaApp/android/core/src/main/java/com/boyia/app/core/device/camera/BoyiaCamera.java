package com.boyia.app.core.device.camera;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Camera;
import android.graphics.ImageFormat;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.CaptureResult;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.OutputConfiguration;
import android.hardware.camera2.params.SessionConfiguration;
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
import com.boyia.app.loader.job.IJob;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.Semaphore;

public class BoyiaCamera {
    private static final String TAG = "BoyiaCamera";
    private static final String CAMERA_ID = "0";
    // 在显示上实时绘制图像，类似视频, 相机使用的纹理
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

    private Semaphore mLock = new Semaphore(1);

    // 视频录制
    private BoyiaCameraRecorder mRecorder;

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

                if (mRecorder != null) {
                    mRecorder.start();
                }
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
            // 创建摄像头预览
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

    public BoyiaCamera() {
        mTexture = BoyiaTexture.createTexture();
    }

    public long getCamerId() {
        return mTexture.getTextureId();
    }

    public void setIsNeedCapture(boolean isNeedCapture) {
        mIsNeedCapture = isNeedCapture;
    }

    boolean configCamera(CameraManager cameraManager, String cameraId) throws CameraAccessException {
        CameraCharacteristics characteristics = cameraManager.getCameraCharacteristics(cameraId);
        StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
        return map != null;
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
    private void initThread() {
        mCameraThread = new HandlerThread(String.format("boyia-camera-%d", mTexture.getTextureId()));
        mCameraHandler = new Handler(mCameraThread.getLooper());
    }

    /**
     * 打开摄像头
     * Call by JNI
     */
    public void openCamera() {
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
            // 打开摄像头, 当摄像头启动时会回调StateCallback，第三个参数是线程的handler
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
                    mImageReader = ImageReader.newInstance(300, 300, ImageFormat.JPEG, 1);
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
            Surface surface = new Surface(mTexture.getSurfaceTexture());
            mPreviewBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
            mPreviewBuilder.addTarget(surface);

            // 创建session
            mCameraDevice.createCaptureSession(
                    Arrays.asList(surface),
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
    }

    private ImageReader.OnImageAvailableListener mOnImageAvailableListener = new ImageReader.OnImageAvailableListener() {
        @Override
        public void onImageAvailable(ImageReader reader) {
            Image image = reader.acquireNextImage();
            if (image == null) {
                return;
            }

            mCameraHandler.post(new BoyiaCameraImageSaver(
                    image,
                    new BoyiaCameraImageSaver.Callback() {
                        @Override
                        public void onComplete(String absolutePath) {
                            // TODO 回调文件地址
                        }

                        @Override
                        public void onError(String errorCode, String errorMessage) {
                        }
                    }));
        }
    };

    /**
     * Call by JNI
     * 获取截图
     * @param autoFocus 是否自动聚焦
     */
    public void takePicture(boolean autoFocus) {
        try {
            mImageReader.setOnImageAvailableListener(mOnImageAvailableListener, mCameraHandler);
            if (autoFocus) {
                mPreviewBuilder.set(
                        CaptureRequest.CONTROL_AF_TRIGGER, CaptureRequest.CONTROL_AF_TRIGGER_START);

                mCaptureSession.capture(mPreviewBuilder.build(), null, mCameraHandler);
            } else {

                mPreviewBuilder.set(
                        CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                        CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_IDLE);
                mCaptureSession.capture(mPreviewBuilder.build(), mCaptureCallback, mCameraHandler);
                mCaptureSession.setRepeatingRequest(
                        mPreviewBuilder.build(), mCaptureCallback, mCameraHandler);

                mPreviewBuilder.set(
                        CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER,
                        CaptureRequest.CONTROL_AE_PRECAPTURE_TRIGGER_START);

                mCaptureSession.capture(mPreviewBuilder.build(), mCaptureCallback, mCameraHandler);
            }

        } catch (CameraAccessException e) {
            BoyiaLog.e(TAG, "takePicture error", e);
        }
    }

    // Call by JNI
    // 开启视频录制
    public void startRecording() {
        mRecorder = new BoyiaCameraRecorder(mCameraId);
        mRecorder.prepare();

        if (mRecorder.getSurface() != null) {
            List<Surface> surfaces = new ArrayList<>();
            surfaces.add(mRecorder.getSurface());
            surfaces.add(mImageReader.getSurface());

            try {
                createCaptureSession(surfaces, CameraDevice.TEMPLATE_RECORD, () -> mRecorder.start());
            } catch (CameraAccessException e) {
                throw new RuntimeException(e);
            }
        }
    }

    // Call by JNI
    public void stopRecording() {
        if (mRecorder != null) {
            mRecorder.stop();
            mRecorder = null;
        }
    }

    // 创建摄像头捕获session，这一步摄像头会开始运行，数据将会输出到surface中
    private void createCaptureSession(List<Surface> remainingSurfaces, int templateType, IJob job) throws CameraAccessException {
        // Close any existing capture session.
        mCaptureSession = null;

        // Create a new capture builder.
        mPreviewBuilder = mCameraDevice.createCaptureRequest(templateType);

        Surface flutterSurface = new Surface(mTexture.getSurfaceTexture());
        mPreviewBuilder.addTarget(flutterSurface);

        if (templateType != CameraDevice.TEMPLATE_PREVIEW) {
            // If it is not preview mode, add all surfaces as targets
            // except the surface used for still capture as this should
            // not be part of a repeating request.
            Surface pictureImageReaderSurface = mImageReader.getSurface();
            for (Surface surface : remainingSurfaces) {
                if (surface == pictureImageReaderSurface) {
                    continue;
                }
                mPreviewBuilder.addTarget(surface);
            }
        }
        // Prepare the callback.
        CameraCaptureSession.StateCallback callback =
                new CameraCaptureSession.StateCallback() {
                    boolean captureSessionClosed = false;

                    @Override
                    public void onConfigured(CameraCaptureSession session) {
                        BoyiaLog.i(TAG, "CameraCaptureSession onConfigured");
                        try {
                            mCaptureSession.setRepeatingRequest(
                                    mPreviewBuilder.build(), mCaptureCallback, mCameraHandler);
                            if (job != null) {
                                job.exec();
                            }
                        } catch (CameraAccessException e) {
                            throw new RuntimeException(e);
                        }
                    }

                    @Override
                    public void onConfigureFailed(CameraCaptureSession cameraCaptureSession) {
                        BoyiaLog.i(TAG, "CameraCaptureSession onConfigureFailed");
                    }

                    @Override
                    public void onClosed(CameraCaptureSession session) {
                        BoyiaLog.i(TAG, "CameraCaptureSession onClosed");
                        captureSessionClosed = true;
                    }
                };

        // Start the session.
        if (BoyiaCameraRecorder.SdkCapabilityChecker.supportsSessionConfiguration()) {
            // Collect all surfaces to render to.
            List<OutputConfiguration> configs = new ArrayList<>();
            configs.add(new OutputConfiguration(flutterSurface));
            for (Surface surface : remainingSurfaces) {
                configs.add(new OutputConfiguration(surface));
            }
            mCameraDevice.createCaptureSession(
                    new SessionConfiguration(
                            SessionConfiguration.SESSION_REGULAR,
                            configs,
                            Executors.newSingleThreadExecutor(),
                            callback));
        } else {
            // Collect all surfaces to render to.
            List<Surface> surfaceList = new ArrayList<>();
            surfaceList.add(flutterSurface);
            surfaceList.addAll(remainingSurfaces);
            mCameraDevice.createCaptureSession(surfaceList, callback, mCameraHandler);
        }
    }

    public void setOnPreviewListener(OnPreviewListener onPreviewListener) {
        this.mOnPreviewListener = onPreviewListener;
    }

    public interface OnPreviewListener {
        void onPreviewFrame(byte[] data, int len);
    }

}
