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
    public ILoadListener mListener;

    public Downloader() {
        this(null);
    }

    public Downloader(ILoadListener listener) {
        mListener = listener;
    }

    public void download(String url) {
        String name = BoyiaUtils.getStringMD5(url);
        String path = DOWN_LOAD_DIR + File.separator + name;

        mInfo = new DownloadData();
        mInfo.setFileUrl(url);
        mInfo.setFilePath(path);
        mInfo.setFileName(name);
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
        File downDir = new File(Downloader.DOWN_LOAD_DIR);
        if (!downDir.exists()) {
            downDir.mkdirs();
        }

        File downFile = new File(mInfo.getFilePath());
        if (downFile.exists()) {
            rangeStart = downFile.length();
        }

        if (mInfo.getStatus() == DownloadData.ERROR) {
            rangeStart = 0;
        }

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

        //DownloadUtil.addDownloadInfo(mInfo);
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
            //DownloadUtil.updateDownloadInfo(mInfo);

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
