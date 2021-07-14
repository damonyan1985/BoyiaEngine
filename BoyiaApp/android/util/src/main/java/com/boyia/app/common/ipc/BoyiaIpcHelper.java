package com.boyia.app.common.ipc;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.RemoteException;

import com.boyia.app.common.ipc.IBoyiaIpcSender.BoyiaSenderStub;
import com.boyia.app.common.utils.BoyiaLog;

public class BoyiaIpcHelper {
    private static final String TAG = "BoyiaIpcHelper";
    public static final String BOYIA_IPC_SENDER = "BoyiaIpcSender";

    /**
     * 通过intent传递自己的binder对象，则无需创建service
     */
    public static void startActivityWithBoyiaSender(
            Activity activity, String action, BoyiaSenderStub binder) {
        Intent intent = new Intent();
        intent.setAction(action);
        if (binder != null) {
            intent.putExtra(BOYIA_IPC_SENDER, new IpcIntentData(binder));
        }
        activity.startActivity(intent);
    }

    /**
     * 通过intent获取ipcsender
     * @param intent
     * @param paramName
     * @return
     */
    public static IBoyiaSender getBoyiaSender(Intent intent, String paramName) {
        IpcIntentData intentData = intent.getParcelableExtra(paramName);
        if (intentData == null) {
            return null;
        }

        return IBoyiaIpcSender.BoyiaSenderStub.asInterface(intentData.asBinder());
    }

    public static class IpcIntentData implements Parcelable {
        private IBinder mBinder;
        public IpcIntentData(IBinder binder) {
            mBinder = binder;
        }

        public IpcIntentData(Parcel parcel) {
            mBinder = parcel.readStrongBinder();
        }

        public IpcIntentData() {
        }

        public IBinder asBinder() {
            return mBinder;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeStrongBinder(mBinder);
        }

        public static final Creator<IpcIntentData> CREATOR = new Creator<IpcIntentData>() {
            @Override
            public IpcIntentData createFromParcel(Parcel source) {
                return new IpcIntentData(source);
            }

            @Override
            public IpcIntentData[] newArray(int size) {
                return new IpcIntentData[size];
            }
        };
    }
}
