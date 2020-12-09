package com.boyia.app.common.ipc;

import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;

/**
 * 远程调用数据类
 */
public class BoyiaIpcData implements Parcelable {
    /**
     * 方法名
     */
    private String mMethod;

    /**
     * 方法参数
     */
    private Bundle mParams;


    public BoyiaIpcData() {
    }

    public BoyiaIpcData(String method, Bundle params) {
        mMethod = method;
        mParams = params;
    }

    public String getMethod() {
        return mMethod;
    }

    public Bundle getParams() {
        return mParams;
    }

    /**
     * 从source中读出方法名与参数
     * @param source
     */
    public BoyiaIpcData(Parcel source) {
        mMethod = source.readString();
        mParams = source.readBundle();
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
        dest.writeString(mMethod);
        dest.writeBundle(mParams);
    }

    public static final Creator<BoyiaIpcData> CREATOR = new Creator<BoyiaIpcData>() {
        @Override
        public BoyiaIpcData createFromParcel(Parcel source) {
            return new BoyiaIpcData(source);
        }

        @Override
        public BoyiaIpcData[] newArray(int size) {
            return new BoyiaIpcData[size];
        }
    };
}
