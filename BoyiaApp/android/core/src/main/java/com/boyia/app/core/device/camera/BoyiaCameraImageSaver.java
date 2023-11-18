package com.boyia.app.core.device.camera;

import android.media.Image;

import com.boyia.app.common.BaseApplication;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class BoyiaCameraImageSaver implements Runnable {
    /** The JPEG image */
    private final Image mImage;

    /** The file we save the image into. */
    private File mCaptureImage;

    /** Used to report the status of the save action. */
    private final Callback mCallback;

    /**
     * Creates an instance of the ImageSaver runnable
     *
     * @param image - The image to save
     * @param file - The file to save the image to
     * @param callback - The callback that is run on completion, or when an error is encountered.
     */
    BoyiaCameraImageSaver(Image image, Callback callback) {
        mImage = image;
        final File outputDir = BaseApplication.getInstance().getCacheDir();
        try {
            mCaptureImage = File.createTempFile("CAP", ".jpg", outputDir);
        } catch (IOException e) {
            mCaptureImage = null;
        }
        mCallback = callback;
    }

    @Override
    public void run() {
        if (mCaptureImage == null) {
            return;
        }
        ByteBuffer buffer = mImage.getPlanes()[0].getBuffer();
        byte[] bytes = new byte[buffer.remaining()];
        buffer.get(bytes);
        FileOutputStream output = null;
        try {
            output = FileOutputStreamFactory.create(mCaptureImage);
            output.write(bytes);

            mCallback.onComplete(mCaptureImage.getAbsolutePath());

        } catch (IOException e) {
            mCallback.onError("IOError", "Failed saving image");
        } finally {
            mImage.close();
            if (null != output) {
                try {
                    output.close();
                } catch (IOException e) {
                    mCallback.onError("cameraAccess", e.getMessage());
                }
            }
        }
    }

    /**
     * The interface for the callback that is passed to ImageSaver, for detecting completion or
     * failure of the image saving task.
     */
    public interface Callback {
        /**
         * Called when the image file has been saved successfully.
         *
         * @param absolutePath - The absolute path of the file that was saved.
         */
        void onComplete(String absolutePath);

        /**
         * Called when an error is encountered while saving the image file.
         *
         * @param errorCode - The error code.
         * @param errorMessage - The human readable error message.
         */
        void onError(String errorCode, String errorMessage);
    }

    /** Factory class that assists in creating a {@link FileOutputStream} instance. */
    static class FileOutputStreamFactory {
        /**
         * Creates a new instance of the {@link FileOutputStream} class.
         *
         * <p>This method is visible for testing purposes only and should never be used outside this *
         * class.
         *
         * @param file - The file to create the output stream for
         * @return new instance of the {@link FileOutputStream} class.
         * @throws FileNotFoundException when the supplied file could not be found.
         */
        public static FileOutputStream create(File file) throws FileNotFoundException {
            return new FileOutputStream(file);
        }
    }
}
