package com.boyia.app.loader.image;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.boyia.app.common.utils.DiskLruCache;
import com.boyia.app.loader.job.IJob;
import com.boyia.app.loader.job.JobScheduler;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.common.utils.FileUtil;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;

/*
 * BoyiaImager
 * @Author Boyia
 * @Date 2012-5-4
 * @Copyright Reserved
 * @Descrption Image Resource Download
 *     Base On IBoyiaImage
 */

public class BoyiaImager {
    private static final String TAG = BoyiaImager.class.getSimpleName();
    protected static final String BOYIA_IMAGE_DISK_SAVE_PATH = BoyiaFileUtil
            .getFilePathRoot() + "boyia_image_cache/";
    private static final int BOYIA_IMAGE_SDCARD_DISK_CACHE_SIZE = 60 * 1024 * 1024;
    private static final int BOYIA_IMAGE_DISK_CACHE_SIZE = 20 * 1024 * 1024;

    private BoyiaImageCache mBitmapCache = null;
    private DiskLruCache mDiskCache = null;
    // Loading Cache
    private HashMap<String, List<WeakReference<IBoyiaImage>>> mLoadingMap;

    private static class BoyiaImageManagerHolder {
        static final BoyiaImager INSTANCE = new BoyiaImager();
    }

    public static BoyiaImager getInstance() {
        return BoyiaImageManagerHolder.INSTANCE;
    }

    private BoyiaImager() {
        initMemoryCache();
        initDiskCache(false);
        mLoadingMap = new HashMap<>();
    }

    public static void loadImage(final String url, final IBoyiaImage image) {
        BoyiaImager.getInstance().asyncloadImage(url, image, true);
    }

    private void asyncloadImage(final String url, final IBoyiaImage image, final boolean needCached) {
        if (url == null) {
            return;
        }

        JobScheduler.getInstance().sendJob(new IJob() {
            @Override
            public void exec() {
                loadImage(url, image, needCached);
            }
        });
    }

    public void putBitmapCache(String url, Bitmap bitmap) {
        if (mBitmapCache == null) {
            return;
        }

        synchronized (mBitmapCache) {
            mBitmapCache.put(url, bitmap);
        }
    }

    protected List<WeakReference<IBoyiaImage>> getImageList(String url) {
        return mLoadingMap == null || mLoadingMap.size() == 0 ? null : mLoadingMap.get(url);
    }

    public void removeLoadImage(String url) {
        if (mLoadingMap == null) {
            return;
        }

        synchronized (mLoadingMap) {
            mLoadingMap.remove(url);
        }
    }

    protected void putLoadImage(String url,
                                List<WeakReference<IBoyiaImage>> imageList) {
        if (mLoadingMap == null) {
            return;
        }

        synchronized (mLoadingMap) {
            mLoadingMap.put(url, imageList);
        }
    }

    private boolean obtainImageFromLoadingMap(IBoyiaImage image, String url) {
        image.setImageURL(url);
        List<WeakReference<IBoyiaImage>> imageList = mLoadingMap.get(url);
        if (imageList == null) {
            imageList = new ArrayList<>();
            imageList.add(new WeakReference<>(image));
            putLoadImage(url, imageList);
        } else {
            imageList.add(new WeakReference<>(image));
            return true;
        }

        return false;
    }

    private boolean obtainImageFromCache(IBoyiaImage image, String url) {
        // First get image in memory cache
        Bitmap bitmap = mBitmapCache.get(url);
        if (bitmap != null) {
            BoyiaLog.d(TAG, "Bitmap in Bitmap Memory Cache url=" + url);
            image.setImage(bitmap);
            return true;
        }

        // Second in disk cache
        bitmap = readDiskCachedBitmap(url, image);
        if (bitmap != null) {
            BoyiaLog.d(TAG, "Bitmap in Disk Cache url=" + url);
            image.setImage(bitmap);
            putBitmapCache(url, bitmap);
            return true;
        }

        return false;
    }

    private void startLoadImage(String url, boolean needLocal) {
        // Get image from network
        new BoyiaImageRunner(url, needLocal).startLoad();
    }

    private void loadImage(String url, IBoyiaImage image, boolean needCached) {
        // First get image in memory cache
        // Second in disk cache
        // Fetch Image from loading queue
        if (image != null && (obtainImageFromCache(image, url) || obtainImageFromLoadingMap(image, url))) {
            return;
        }

        BoyiaLog.d(TAG,
                "Bitmap is not in any cache, begin to load in network and url is " + url);
        // Create a new loading
        startLoadImage(url, needCached);
    }

    // Init memory cache
    private void initMemoryCache() {
        if (mBitmapCache == null) {
            int maxMemory = (int) (Runtime.getRuntime().maxMemory() / 1024); // KB
            mBitmapCache = new BoyiaImageCache(maxMemory / 4);
        }
    }

    private void initDiskCache(boolean recreate) {
        if (!BoyiaUtils.existSDCard()) {
            return;
        }

        if (mDiskCache == null) {
            long cacheSize = Environment.MEDIA_MOUNTED.equals(Environment
                    .getExternalStorageState()) ? BOYIA_IMAGE_SDCARD_DISK_CACHE_SIZE
                    : BOYIA_IMAGE_DISK_CACHE_SIZE;
            try {
                // Clear Cache
                if (recreate) {
                    FileUtil.deleteFolder(BOYIA_IMAGE_DISK_SAVE_PATH);
                }

                File cacheDir = BoyiaFileUtil.createDirectory(BOYIA_IMAGE_DISK_SAVE_PATH);
                if (cacheDir == null || !cacheDir.exists()) {
                    return;
                }

                BoyiaLog.d(TAG, "The DistCacheDir is " + cacheDir.getAbsolutePath());

                mDiskCache = DiskLruCache.open(cacheDir, 1, 1, cacheSize);
            } catch (IOException e) {
                e.printStackTrace();
                mDiskCache = null;
            }
        }
    }

    private Bitmap readDiskCachedBitmap(String url, IBoyiaImage image) {
        if (mDiskCache == null) {
            return null;
        }

        DiskLruCache.Snapshot snapshot;
        Bitmap bitmap = null;
        String key = Integer.toHexString(url.hashCode());
        try {
            synchronized (mDiskCache) {
                snapshot = mDiskCache.get(key);
                if (snapshot != null) {
                    BoyiaLog.d(TAG, "Bitmap url=" + image.getImageURL());
                    BitmapFactory.Options options = new BitmapFactory.Options();
                    options.inJustDecodeBounds = true;
                    BitmapFactory.decodeStream(snapshot.getInputStream(0), null, options);
                    options.inJustDecodeBounds = false;
                    options.inSampleSize = options.outWidth / image.getWidth();
                    BoyiaLog.d(TAG, "Bitmap Size=" + options.inSampleSize + " Image url=" + image.getImageURL());
                    snapshot.close();
                    snapshot = mDiskCache.get(key);
                    bitmap = BitmapFactory.decodeStream(snapshot.getInputStream(0), null, options);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            BoyiaLog.d(TAG, "Bitmap Error=" + e.toString());
            bitmap = null;
        }

        return bitmap;
    }

    public void recycle() {
        if (mBitmapCache == null) {
            return;
        }

        synchronized (mBitmapCache) {
            mBitmapCache.evictAll();
        }
    }

    protected void cacheDisk(String url, byte[] data) {
        if (mDiskCache == null) {
            return;
        }

        DiskLruCache.Editor editor = null;
        String key = Integer.toHexString(url.hashCode());
        try {
            synchronized (mDiskCache) {
                editor = mDiskCache.edit(key);
                OutputStream output = editor.newOutputStream(0);
                output.write(data);
                editor.commit();
                mDiskCache.flush();
                output.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
            if (editor != null) {
                try {
                    editor.abort();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }
        }
    }

    public void clearMemoryCache() {
        mBitmapCache.evictAll();
    }
}
