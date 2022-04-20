package com.boyia.app.shell.update;

import java.io.File;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.List;

import com.boyia.app.common.utils.BoyiaUtils;
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
    private static final String TAG = "Downloader";
    public static final String DOWN_LOAD_DIR = BoyiaFileUtil.getFilePathRoot() + "download";
    private BoyiaLoader mLoader;
    private DownloadData mInfo;
    private RandomAccessFile mSavedFile;
    private DownLoadProgressListener mListener;
    private String mFileDir;
    private String mUrl;

    public Downloader() {
        this(null, null);
    }

    public Downloader(DownLoadProgressListener listener, String fileDir) {
        mListener = listener;
        mFileDir = BoyiaUtils.isTextEmpty(fileDir) ? DOWN_LOAD_DIR : (fileDir + "download");
    }

    public void download(String url) {
        mUrl = url;
        mLoader = new BoyiaLoader(this);
        BoyiaLog.d(TAG, "download url="+url);
        mLoader.load(url);
    }

    public boolean initDownloadData(String url) {
        String name = BoyiaUtils.getStringMD5(url);
        String path = mFileDir + File.separator + name;

        DownloadData info = new DownloadData();
        info.setFileName(name);

        // 查询是否已经存在同名数据
        List<DownloadData> list = DownloadUtil.getDownloadList(info);
        if (list == null || list.size() == 0) {
            info.setFilePath(path);
            info.setFileUrl(url);
            mInfo = info;
            return true;
        }

        BoyiaLog.d(TAG, "download getDownloadList size = " + list.size());
        mInfo = list.get(0);
        if (mInfo.getStatus() != DownloadData.FINISHED) {
            return true;
        }

        // 如果下载完成，对比文件MD5值与数据库中是否一致
        File file = new File(mInfo.getFilePath());
        if (!file.exists()) {
            return true;
        }

        String md5 = BoyiaUtils.getFileMD5(file);
        BoyiaLog.d(TAG, "initDownloadData md5 = " + md5);
        if (!mInfo.getFileMd5().equals(md5)) {
            return true;
        }

        // 如果下载完了，任务结束，不再继续执行下去
        if (mListener != null) {
            mListener.onCompleted();
        }

        return false;
    }

    @Override
    public boolean onLoadStart() {
        // 初始化下载数据
        if (!initDownloadData(mUrl)) {
            return false;
        }

        long rangeStart = 0;
        File downDir = new File(mFileDir);
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
            return false;
        }

        // 正式开始启动下载
        mInfo.setStatus(DownloadData.DOWNLOADING);

        if (mInfo.getId() == null || mInfo.getId() == 0) {
            DownloadUtil.addDownloadInfo(mInfo);
        } else {
            DownloadUtil.updateDownloadInfo(mInfo);
            if (mListener != null) {
                mListener.onProgress(mInfo.getCurrentSize(), mInfo.getMaxLength());
            }
        }

        return true;
    }

    @Override
    public void onLoadDataSize(long size) {
        BoyiaLog.d(TAG, "download size="+size);
        mInfo.setMaxLength(size);
    }

    @Override
    public void onLoadDataReceive(byte[] data, int length, Object msg) {
        try {
            mSavedFile.write(data, 0, length);
            mInfo.setCurrentSize(mInfo.getCurrentSize() + length);
            if (mListener != null) {
                mListener.onProgress(mInfo.getCurrentSize(), mInfo.getMaxLength());
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void onLoadFinished(Object msg) {
        try {
            String md5 = BoyiaUtils.getFileMD5(new File(mInfo.getFilePath()));
            mSavedFile.close();

            mInfo.setFileMd5(md5);
            mInfo.setStatus(DownloadData.FINISHED);
            DownloadUtil.updateDownloadInfo(mInfo);

            if (mListener != null) {
                mListener.onCompleted();
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

    public interface DownLoadProgressListener {
        void onProgress(long current, long size);
        void onCompleted();
    }
}
