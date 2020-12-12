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

    public static BoyiaSenderStub createSenderStub(IBoyiaSender proxy) {
        return new BoyiaSenderStub() {
            @Override
            public BoyiaIpcData sendMessageSync(BoyiaIpcData message) throws RemoteException {
                BoyiaLog.i(TAG, "sendMessageSync method=" + message.getMethod());
                if (proxy != null) {
                    return proxy.sendMessageSync(message);
                }

                return null;
            }

            @Override
            public void sendMessageAsync(BoyiaIpcData message, IBoyiaIpcCallback callback) throws RemoteException {
                BoyiaLog.i(TAG, "sendMessageSync method=" + message.getMethod());
                if (proxy != null) {
                    proxy.sendMessageAsync(message, callback);
                }
            }
        };
    }

    /**
     * 客户端调用，获取IBoyiaSender对象
     * @param context
     * @param callback
     */
    public static ServiceConnection bindService(Context context, BoyiaIpcService.BoyiaIpcBindCallback callback) {
        ServiceConnection connection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder binder) {
                callback.callback(IBoyiaIpcSender.BoyiaSenderStub.asInterface(binder));
            }

            @Override
            public void onServiceDisconnected(ComponentName name) {
            }
        };
        context.bindService(new Intent(context, BoyiaIpcService.class), connection, Context.BIND_AUTO_CREATE);

        return connection;
    }

    /**
     * 通过intent传递自己的binder对象，则无需创建service
     */
    public static void startActivityWithBoyiaSender(
            Activity activity, String action,
            String paramName, BoyiaSenderStub binder) {
        Intent intent = new Intent();
        intent.setAction(action);
        intent.putExtra(paramName, new IpcIntentData(binder));
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
