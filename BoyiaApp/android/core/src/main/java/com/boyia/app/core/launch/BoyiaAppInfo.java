package com.boyia.app.core.launch;

import android.os.IBinder;
import android.os.Parcel;
import android.os.Parcelable;

public class BoyiaAppInfo implements Parcelable {
    public int mAppId;
    public int mAppVersion;
    public String mAppName;
    public String mAppPath;
    public String mAppUrl;
    public String mAppCover;
    public IBinder mHostBinder;

    public BoyiaAppInfo(int appId, int version, String name, String path, String url, String cover, IBinder binder) {
        mAppId = appId;
        mAppVersion = version;
        mAppName = name;
        mAppPath = path;
        mAppUrl = url;
        mAppCover = cover;
        mHostBinder = binder;
    }

    public BoyiaAppInfo(Parcel source) {
        mAppId = source.readInt();
        mAppVersion = source.readInt();
        mAppName = source.readString();
        mAppPath = source.readString();
        mAppUrl = source.readString();
        mAppCover = source.readString();
        mHostBinder = source.readStrongBinder();
    }

    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * 写入数据
     * @param dest
     * @param flags
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(mAppId);
        dest.writeInt(mAppVersion);
        dest.writeString(mAppName);
        dest.writeString(mAppPath);
        dest.writeString(mAppUrl);
        dest.writeString(mAppCover);
        dest.writeStrongBinder(mHostBinder);
    }

    @Override
    public String toString() {
        return "BoyiaAppInfo{" +
                "mAppId=" + mAppId +
                ", mAppVersion=" + mAppVersion +
                ", mAppName='" + mAppName + '\'' +
                ", mAppPath='" + mAppPath + '\'' +
                ", mAppUrl='" + mAppUrl + '\'' +
                ", mAppCover='" + mAppCover + '\'' +
                '}';
    }

    public static final Parcelable.Creator<BoyiaAppInfo> CREATOR = new Parcelable.Creator<BoyiaAppInfo>() {
        @Override
        public BoyiaAppInfo createFromParcel(Parcel source) {
            return new BoyiaAppInfo(source);
        }

        @Override
        public BoyiaAppInfo[] newArray(int size) {
            return new BoyiaAppInfo[size];
        }
    };
}
