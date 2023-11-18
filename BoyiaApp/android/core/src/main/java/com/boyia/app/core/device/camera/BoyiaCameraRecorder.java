package com.boyia.app.core.device.camera;

import android.annotation.TargetApi;
import android.media.CamcorderProfile;
import android.media.EncoderProfiles;
import android.media.MediaRecorder;
import android.os.Build;
import android.view.Surface;

import com.boyia.app.common.BaseApplication;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.core.BoyiaBridge;

import java.io.File;
import java.io.IOException;

public class BoyiaCameraRecorder {
    private static final String TAG = "BoyiaCameraRecorder";

    private MediaRecorder mRecorder;

    private EncoderProfiles mEncoderProfiles;

    private CamcorderProfile mCamcorderProfile;

    private File mCaptureFile;

    private int mCamerId;

    public BoyiaCameraRecorder(String camerId) {
        mCamerId = Integer.parseInt(camerId);
    }

    public void prepare() {
        final File outputDir = BaseApplication.getInstance().getCacheDir();
        try {
            mCaptureFile = File.createTempFile("REC", ".mp4", outputDir);
        } catch (IOException | SecurityException e) {
            BoyiaLog.e(TAG, "prepare error", e);
            return;
        }

        prepareMediaRecorder(mCaptureFile.getAbsolutePath());
    }

    public void start() {
        if (mRecorder != null) {
            mRecorder.start();
        }
    }

    public void stop() {
        if (mRecorder != null) {
            mRecorder.stop();
            mRecorder.reset();
        }
    }

    public Surface getSurface() {
        return mRecorder != null ? mRecorder.getSurface() : null;
    }
    /**
     * 准备录制
     * @param outputFilePath
     */
    private void prepareMediaRecorder(String outputFilePath) {
        if (mRecorder != null) {
            mRecorder.release();
        }

        BoyiaCameraResolution resolution = BoyiaCameraResolution.high;
        if (mEncoderProfiles == null) {
            mEncoderProfiles = getEncoderProfiles(mCamerId, resolution);
        }
        Builder builder;
        if (SdkCapabilityChecker.supportsEncoderProfiles() && mEncoderProfiles != null) {
            builder = new Builder(mEncoderProfiles, outputFilePath);
        } else {
            if (mCamcorderProfile == null) {
                mCamcorderProfile = getCamcorderProfile(mCamerId, resolution);
            }
            builder = new Builder(mCamcorderProfile, outputFilePath);
        }

        try {
            builder.setEnableAudio(true);
            mRecorder = builder.build();
        } catch (IOException e) {
            BoyiaLog.e(TAG, "create media recorder error", e);
            mRecorder = null;
        }
    }

    public static CamcorderProfile getCamcorderProfile(
            int cameraId, BoyiaCameraResolution preset) {
        if (cameraId < 0) {
            throw new AssertionError(
                    "getCamcorderProfile can only be used with valid (>=0) camera identifiers.");
        }

        switch (preset) {
            case max:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_HIGH)) {
                    return CamcorderProfile.get(cameraId, CamcorderProfile.QUALITY_HIGH);
                }
                // fall through
            case ultraHigh:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_2160P)) {
                    return CamcorderProfile.get(cameraId, CamcorderProfile.QUALITY_2160P);
                }
                // fall through
            case veryHigh:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_1080P)) {
                    return CamcorderProfile.get(cameraId, CamcorderProfile.QUALITY_1080P);
                }
                // fall through
            case high:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_720P)) {
                    return CamcorderProfile.get(cameraId, CamcorderProfile.QUALITY_720P);
                }
                // fall through
            case medium:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_480P)) {
                    return CamcorderProfile.get(cameraId, CamcorderProfile.QUALITY_480P);
                }
                // fall through
            case low:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_QVGA)) {
                    return CamcorderProfile.get(cameraId, CamcorderProfile.QUALITY_QVGA);
                }
                // fall through
            default:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_LOW)) {
                    return CamcorderProfile.get(cameraId, CamcorderProfile.QUALITY_LOW);
                } else {
                    throw new IllegalArgumentException(
                            "No capture session available for current capture session.");
                }
        }
    }

    @TargetApi(Build.VERSION_CODES.S)
    public static EncoderProfiles getEncoderProfiles(
            int cameraId, BoyiaCameraResolution preset) {
        if (cameraId < 0) {
            throw new AssertionError(
                    "getEncoderProfiles can only be used with valid (>=0) camera identifiers.");
        }

        String cameraIdString = Integer.toString(cameraId);

        switch (preset) {
            case max:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_HIGH)) {
                    return CamcorderProfile.getAll(cameraIdString, CamcorderProfile.QUALITY_HIGH);
                }
                // fall through
            case ultraHigh:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_2160P)) {
                    return CamcorderProfile.getAll(cameraIdString, CamcorderProfile.QUALITY_2160P);
                }
                // fall through
            case veryHigh:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_1080P)) {
                    return CamcorderProfile.getAll(cameraIdString, CamcorderProfile.QUALITY_1080P);
                }
                // fall through
            case high:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_720P)) {
                    return CamcorderProfile.getAll(cameraIdString, CamcorderProfile.QUALITY_720P);
                }
                // fall through
            case medium:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_480P)) {
                    return CamcorderProfile.getAll(cameraIdString, CamcorderProfile.QUALITY_480P);
                }
                // fall through
            case low:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_QVGA)) {
                    return CamcorderProfile.getAll(cameraIdString, CamcorderProfile.QUALITY_QVGA);
                }
                // fall through
            default:
                if (CamcorderProfile.hasProfile(cameraId, CamcorderProfile.QUALITY_LOW)) {
                    return CamcorderProfile.getAll(cameraIdString, CamcorderProfile.QUALITY_LOW);
                }

                throw new IllegalArgumentException(
                        "No capture session available for current capture session.");
        }
    }

    public static class Builder {
        static class MediaRecorderFactory {
            MediaRecorder makeMediaRecorder() {
                return new MediaRecorder();
            }
        }

        private final String outputFilePath;
        private final CamcorderProfile camcorderProfile;
        private final EncoderProfiles encoderProfiles;
        private final MediaRecorderFactory recorderFactory;

        private boolean enableAudio;
        private int mediaOrientation;

        public Builder(
             CamcorderProfile camcorderProfile, String outputFilePath) {
            this(camcorderProfile, outputFilePath, new MediaRecorderFactory());
        }

        public Builder(
            EncoderProfiles encoderProfiles, String outputFilePath) {
            this(encoderProfiles, outputFilePath, new MediaRecorderFactory());
        }

        Builder(
            CamcorderProfile camcorderProfile,
            String outputFilePath,
            MediaRecorderFactory helper) {
            this.outputFilePath = outputFilePath;
            this.camcorderProfile = camcorderProfile;
            this.encoderProfiles = null;
            this.recorderFactory = helper;
        }

        Builder(
            EncoderProfiles encoderProfiles,
            String outputFilePath,
            MediaRecorderFactory helper) {
            this.outputFilePath = outputFilePath;
            this.encoderProfiles = encoderProfiles;
            this.camcorderProfile = null;
            this.recorderFactory = helper;
        }

        public Builder setEnableAudio(boolean enableAudio) {
            this.enableAudio = enableAudio;
            return this;
        }

        public Builder setMediaOrientation(int orientation) {
            this.mediaOrientation = orientation;
            return this;
        }

        public MediaRecorder build() throws IOException, NullPointerException, IndexOutOfBoundsException {
            MediaRecorder mediaRecorder = recorderFactory.makeMediaRecorder();

            // There's a fixed order that mediaRecorder expects. Only change these functions accordingly.
            // You can find the specifics here: https://developer.android.com/reference/android/media/MediaRecorder.
            if (enableAudio) mediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
            mediaRecorder.setVideoSource(MediaRecorder.VideoSource.SURFACE);

            if (SdkCapabilityChecker.supportsEncoderProfiles() && encoderProfiles != null) {
                EncoderProfiles.VideoProfile videoProfile = encoderProfiles.getVideoProfiles().get(0);
                EncoderProfiles.AudioProfile audioProfile = encoderProfiles.getAudioProfiles().get(0);

                mediaRecorder.setOutputFormat(encoderProfiles.getRecommendedFileFormat());
                if (enableAudio) {
                    mediaRecorder.setAudioEncoder(audioProfile.getCodec());
                    mediaRecorder.setAudioEncodingBitRate(audioProfile.getBitrate());
                    mediaRecorder.setAudioSamplingRate(audioProfile.getSampleRate());
                }
                mediaRecorder.setVideoEncoder(videoProfile.getCodec());
                mediaRecorder.setVideoEncodingBitRate(videoProfile.getBitrate());
                mediaRecorder.setVideoFrameRate(videoProfile.getFrameRate());
                mediaRecorder.setVideoSize(videoProfile.getWidth(), videoProfile.getHeight());
            } else if (camcorderProfile != null) {
                mediaRecorder.setOutputFormat(camcorderProfile.fileFormat);
                if (enableAudio) {
                    mediaRecorder.setAudioEncoder(camcorderProfile.audioCodec);
                    mediaRecorder.setAudioEncodingBitRate(camcorderProfile.audioBitRate);
                    mediaRecorder.setAudioSamplingRate(camcorderProfile.audioSampleRate);
                }
                mediaRecorder.setVideoEncoder(camcorderProfile.videoCodec);
                mediaRecorder.setVideoEncodingBitRate(camcorderProfile.videoBitRate);
                mediaRecorder.setVideoFrameRate(camcorderProfile.videoFrameRate);
                mediaRecorder.setVideoSize(
                        camcorderProfile.videoFrameWidth, camcorderProfile.videoFrameHeight);
            }

            mediaRecorder.setOutputFile(outputFilePath);
            mediaRecorder.setOrientationHint(this.mediaOrientation);

            mediaRecorder.prepare();

            return mediaRecorder;
        }
    }

    public static class SdkCapabilityChecker {
        public static int SDK_VERSION = Build.VERSION.SDK_INT;

        public static boolean supportsDistortionCorrection() {
            return SDK_VERSION >= Build.VERSION_CODES.P;
        }

        public static boolean supportsEglRecordableAndroid() {
            return SDK_VERSION >= Build.VERSION_CODES.O;
        }

        public static boolean supportsEncoderProfiles() {
            return SDK_VERSION >= Build.VERSION_CODES.S;
        }

        public static boolean supportsMarshmallowNoiseReductionModes() {
            return SDK_VERSION >= Build.VERSION_CODES.M;
        }

        public static boolean supportsSessionConfiguration() {
            return SDK_VERSION >= Build.VERSION_CODES.P;
        }

        public static boolean supportsVideoPause() {
            return SDK_VERSION >= Build.VERSION_CODES.N;
        }

        public static boolean supportsZoomRatio() {
            return SDK_VERSION >= Build.VERSION_CODES.R;
        }
    }
}
