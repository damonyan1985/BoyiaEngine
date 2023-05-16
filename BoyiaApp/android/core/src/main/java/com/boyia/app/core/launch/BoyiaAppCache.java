package com.boyia.app.core.launch;

import android.util.LruCache;

/**
 * Key: appName
 * Value: BoyiaAppInfo
 * 使用最近最少使用算法来复用app进程
 */
public class BoyiaAppCache extends LruCache<Integer, BoyiaAppCache.BoyiaAppCacheInfo> {
    private AppProgressReuseListener mListener;
    public BoyiaAppCache(int maxSize, AppProgressReuseListener listener) {
        super(maxSize);
        mListener = listener;
    }

    // 每个单位大小为1
    @Override
    protected int sizeOf(Integer appId, BoyiaAppCache.BoyiaAppCacheInfo value) {
        return 1;
    }

    @Override
    protected void entryRemoved(boolean evicted, Integer appId, BoyiaAppCache.BoyiaAppCacheInfo oldValue, BoyiaAppCache.BoyiaAppCacheInfo newValue) {
        if (oldValue == null || newValue == null) {
            return;
        }

        // key不同，则进行进程复用
        if (!oldValue.appInfo.mAppName.equals(newValue.appInfo.mAppName)) {
            newValue.progressEnd = oldValue.progressEnd;
            mListener.onProgressReuse(newValue);
        }
    }

    public interface AppProgressReuseListener {
        void onProgressReuse(BoyiaAppCache.BoyiaAppCacheInfo cacheInfo);
    }

    public static class BoyiaAppCacheInfo {
        BoyiaAppInfo appInfo;
        String progressEnd;

        public BoyiaAppCacheInfo(BoyiaAppInfo appInfo, String progressEnd) {
            this.appInfo = appInfo;
            this.progressEnd = progressEnd;
        }
    }
}
