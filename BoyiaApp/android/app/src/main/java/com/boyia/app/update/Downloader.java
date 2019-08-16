package com.boyia.app.update;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import com.boyia.app.loader.BoyiaLoader;
import com.boyia.app.loader.ILoadListener;
import com.boyia.app.common.utils.BoyiaFileUtil;
import com.boyia.app.common.utils.BoyiaLog;


/*
 * Downloader
 * @Author Yan bo
 * @Time 2018-12-31
 * @Copyright Reserved
 * @Descrption HTTP Large Resource Download Loader
 *  Interface
 */

public class Downloader implements ILoadListener {
    public static final String DOWN_LOAD_DIR = BoyiaFileUtil.getFilePathRoot() + "download/";
    private BoyiaLoader mLoader;
    private DownloadData mInfo;
    private RandomAccessFile mSavedFile;

    public Downloader() {
    }

    public void download(DownloadData info) {
        mInfo = info;
        mLoader = new BoyiaLoader(this, true);
        BoyiaLog.d("yanbo", "download url="+info.getFileUrl());
        mLoader.load(info.getFileUrl());
    }

    @Override
    public void onLoaderStart() {
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
            mLoader.putHeader("Range", "bytes="+rangeStart+"-");
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
    }

    @Override
    public void onLoaderDataSize(long size) {
        BoyiaLog.d("yanbo", "download size="+size);
        mInfo.setMaxLength(size);
    }

    @Override
    public void onLoaderDataReceive(byte[] data, int length, Object msg) {
        try {
            mSavedFile.write(data, 0, length);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onLoaderFinished(Object msg) {
        try {
            mSavedFile.close();
            mInfo.setStatus(DownloadData.FINISHED);
            DownloadUtil.updateDownloadInfo(mInfo);
            BoyiaLog.d("yanbo", "download onLoaderFinished");
        } catch (IOException e) {
            e.printStackTrace();
        }

        mLoader = null;
    }

    @Override
    public void onLoaderError(String error, Object msg) {
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
    public void onLoaderRedirectUrl(String redirectUrl) {
    }

}
