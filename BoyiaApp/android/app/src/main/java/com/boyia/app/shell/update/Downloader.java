package com.boyia.app.shell.update;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.security.MessageDigest;

import com.boyia.app.common.utils.BoyiaUtils;
import com.boyia.app.loader.BoyiaLoader;
import com.boyia.app.loader.ILoadListener;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.utils.BoyiaLog;
import com.boyia.app.shell.client.BoyiaSimpleLoaderListener;


/*
 * Downloader
 * @Author Yan bo
 * @Time 2018-12-31
 * @Copyright Reserved
 * @Descrption HTTP Large Resource Download Loader
 *  Interface
 */

public class Downloader implements ILoadListener {
    private static final String TAG = "Downloader";
    public static final String DOWN_LOAD_DIR = BoyiaFileUtil.getFilePathRoot() + "download";
    private BoyiaLoader mLoader;
    private DownloadData mInfo;
    private RandomAccessFile mSavedFile;
    private ILoadListener mListener;
    private String mFilePath;


    public Downloader() {
        this(null, null);
    }

    public Downloader(ILoadListener listener, String filePath) {
        mListener = listener;
        mFilePath = BoyiaUtils.isTextEmpty(filePath) ? DOWN_LOAD_DIR : (filePath + "download");
    }

    public void download(String url) {
        download(url, false);
    }

    public void download(String url, boolean useCache) {
        String name = BoyiaUtils.getStringMD5(url);
        String path = mFilePath + File.separator + name;

        mInfo = new DownloadData();
        mInfo.setFileUrl(url);
        mInfo.setFilePath(path);
        mInfo.setFileName(name);

        if (useCache) {
            // TODO 根据name查询表
        }
        download(mInfo);
    }

    public void download(DownloadData info) {
        mInfo = info;
        mLoader = new BoyiaLoader(this);
        BoyiaLog.d(TAG, "download url="+info.getFileUrl());
        mLoader.load(info.getFileUrl());
    }

    @Override
    public void onLoadStart() {
        long rangeStart = 0;
        File downDir = new File(mFilePath);
        if (!downDir.exists()) {
            downDir.mkdirs();
        }

        File downFile = new File(mInfo.getFilePath());
        if (downFile.exists()) {
            rangeStart = downFile.length();
        }

        if (mInfo.getStatus() == null || mInfo.getStatus() == DownloadData.ERROR) {
            rangeStart = 0;
            mInfo.setMaxLength(0L);
            mInfo.setStatus(DownloadData.PAUSE);
        }

        if (rangeStart >= mInfo.getMaxLength() && downFile.exists()) {
            downFile.delete();
            rangeStart = 0;
            mInfo.setMaxLength(0L);
        }

        mInfo.setCurrentSize(rangeStart);

        // 断点续传
        if (rangeStart != 0) {
            mLoader.putRequestHeader("Range", "bytes="+rangeStart+"-");
        }

        try {
            mSavedFile = new RandomAccessFile(mInfo.getFilePath(), "rw");
            mSavedFile.seek(rangeStart);
        } catch (Exception e) {
            e.printStackTrace();
            return;
        }

        // 正式开始启动下载
        mInfo.setStatus(DownloadData.DOWNLOADING);

        DownloadUtil.addDownloadInfo(mInfo);
    }

    @Override
    public void onLoadDataSize(long size) {
        BoyiaLog.d(TAG, "download size="+size);
        mInfo.setMaxLength(size);
        if (mListener != null) {
            mListener.onLoadDataSize(size);
        }
    }

    @Override
    public void onLoadDataReceive(byte[] data, int length, Object msg) {
        try {
            mSavedFile.write(data, 0, length);
            mInfo.setCurrentSize(mInfo.getCurrentSize() + length);
            if (mListener != null) {
                mListener.onLoadDataReceive(data, length, msg);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onLoadFinished(Object msg) {
        try {
            mSavedFile.close();
            mInfo.setStatus(DownloadData.FINISHED);
            DownloadUtil.updateDownloadInfo(mInfo);

            if (mListener != null) {
                mListener.onLoadFinished(msg);
            }
            BoyiaLog.d(TAG, "download onLoaderFinished");
        } catch (IOException e) {
            e.printStackTrace();
        }

        mLoader = null;
    }

    @Override
    public void onLoadError(String error, Object msg) {
        if (mSavedFile != null) {
            try {
                mSavedFile.close();
            } catch (IOException ioe) {
                ioe.printStackTrace();
            }

            mInfo.setStatus(DownloadData.ERROR);
            DownloadUtil.updateDownloadInfo(mInfo);
        }

        mLoader = null;
    }

    @Override
    public void onLoadRedirectUrl(String redirectUrl) {
    }
}
