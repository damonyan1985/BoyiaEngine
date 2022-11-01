package com.boyia.app.common.ipc;

import android.os.Binder;
import android.os.IBinder;
import android.os.IInterface;
import android.os.Parcel;
import android.os.Parcelable;
import android.os.RemoteException;

import com.boyia.app.common.utils.BoyiaLog;

public interface IBoyiaIpcSender extends IBoyiaSender, IInterface {
    String TAG = "IBoyiaIpcSender";
    String DESCRIPTOR = "com.boyia.app.common.ipc.IBoyiaSender";
    int SEND_RESULT_NULL_CODE = 0;
    int SEND_RESULT_NOT_NULL_CODE = 1;

    int SEND_MESSAGE_SYNC = IBinder.FIRST_CALL_TRANSACTION;
    int SEND_MESSAGE_ASYNC = IBinder.FIRST_CALL_TRANSACTION + 1;

    /**
     * 服务端调用，BoyiaSenderStub是一个binder
     * binder构造函数会使用native方法创建底层binder
     */
    abstract class BoyiaSenderStub extends Binder implements IBoyiaIpcSender {
        public BoyiaSenderStub() {
            this.attachInterface(this, DESCRIPTOR);
        }

        // 传入的binder是一个远端binder
        public static IBoyiaIpcSender asInterface(IBinder binder) {
            if (binder == null) {
                return null;
            }

            // 如果是同一进程存在该服务，则sender不为空
            IInterface sender = binder.queryLocalInterface(DESCRIPTOR);
            if (sender instanceof IBoyiaIpcSender) {
                return (IBoyiaIpcSender) sender;
            }

            // 如果是不同进程，则使用proxy
            return new BoyiaSenderProxy(binder);
        }

        @Override
        public IBinder asBinder() {
            return this;
        }

        /**
         * 接收IPC消息
         */
        @Override
        public boolean onTransact(int code, Parcel data, Parcel reply,
                                  int flags) throws RemoteException {
            switch (code) {
                case INTERFACE_TRANSACTION: {
                    reply.writeString(DESCRIPTOR);
                    return true;
                }
                case SEND_MESSAGE_SYNC: {
                    data.enforceInterface(DESCRIPTOR);
                    BoyiaIpcData ipcData = BoyiaIpcData.CREATOR.createFromParcel(data);
                    // 服务端需要实现sendMessageSync接口
                    BoyiaIpcData result = this.sendMessageSync(ipcData);
                    reply.writeNoException();
                    if (result != null) {
                        // 1表示数据不为null
                        reply.writeInt(SEND_RESULT_NOT_NULL_CODE);
                        result.writeToParcel(reply,
                                Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
                    } else {
                        reply.writeInt(SEND_RESULT_NULL_CODE);
                    }
                    return true;
                }
                case SEND_MESSAGE_ASYNC: {
                    data.enforceInterface(DESCRIPTOR);
                    BoyiaIpcData ipcData = BoyiaIpcData.CREATOR.createFromParcel(data);
                    // 服务端需要实现sendMessageAsync接口，此时服务端sendMessageAsync内部实现还是采用同步
                    // 改调用会在binder内部线程池中进行调用，不会阻塞UI线程
                    this.sendMessageAsync(ipcData, new IBoyiaIpcCallback() {
                        @Override
                        public void callback(BoyiaIpcData message) {
                            reply.writeNoException();
                            if (message != null) {
                                // 1表示数据不为null
                                reply.writeInt(SEND_RESULT_NOT_NULL_CODE);
                                message.writeToParcel(reply,
                                        Parcelable.PARCELABLE_WRITE_RETURN_VALUE);
                            } else {
                                reply.writeInt(SEND_RESULT_NULL_CODE);
                            }
                        }

                        @Override
                        public IpcScheduler scheduler() {
                            return null;
                        }
                    });

                    return true;
                }
            }
            return super.onTransact(code, data, reply, flags);
        }
    }

    /**
     * 由客户端调用
     */
    class BoyiaSenderProxy implements IBoyiaIpcSender {
        private IBinder mRemote;

        public BoyiaSenderProxy(IBinder binder) {
            mRemote = binder;
        }

        @Override
        public IBinder asBinder() {
            return mRemote;
        }

        /**
         * 同步发消息
         */
        @Override
        public BoyiaIpcData sendMessageSync(BoyiaIpcData message) throws RemoteException {
            return sendMessageImpl(message, SEND_MESSAGE_SYNC);
        }

        /**
         * 异步发消息
         */
        @Override
        public void sendMessageAsync(BoyiaIpcData message, IBoyiaIpcCallback callback) throws RemoteException {
            if (callback == null || callback.scheduler() == null) {
                return;
            }
            // 使用callback所需要的scheduler执行
            callback.scheduler().run(() -> {
                try {
                    callback.callback(sendMessageImpl(message, SEND_MESSAGE_ASYNC));
                } catch (RemoteException e) {
                    e.printStackTrace();
                    BoyiaLog.e(TAG, "BoyiaSenderProxy sendMessageAsync error", e);
                }
            });
        }

        private BoyiaIpcData sendMessageImpl(BoyiaIpcData message, int code) throws RemoteException {
            // 如果mRemote为空
            if (mRemote == null) {
                return null;
            }

            Parcel data = Parcel.obtain();
            Parcel reply = Parcel.obtain();
            BoyiaIpcData result = null;
            try {
                data.writeInterfaceToken(DESCRIPTOR);
                message.writeToParcel(data, 0);
                //mRemote.transact(code, data, reply, code == SEND_MESSAGE_ASYNC ? 1 : 0);
                mRemote.transact(code, data, reply, 0);
                reply.readException();
                // 非0表示数据不为null
                if (reply.readInt() != SEND_RESULT_NULL_CODE) {
                    result = BoyiaIpcData.CREATOR.createFromParcel(reply);
                }

                BoyiaLog.d(TAG, "sendMessageImpl received finished");
            } finally {
                reply.recycle();
                data.recycle();
            }
            return result;
        }

        @Override
        public void whileSenderEnd(IBoyiaSenderListener listener) {
            BoyiaLog.d(TAG, "call whileSenderEnd");
            if (mRemote == null || listener == null) {
                return;
            }

            try {
                BoyiaLog.d(TAG, "add death listener");
                mRemote.linkToDeath(new BoyiaSenderDeathRecipient(listener, mRemote), 0);
            } catch (RemoteException e) {
                BoyiaLog.e(TAG, "linkToDeath error", e);
            }
        }
    }

    class BoyiaSenderDeathRecipient implements IBinder.DeathRecipient {
        private IBoyiaSenderListener mListener;
        private IBinder mRemote;

        public BoyiaSenderDeathRecipient(IBoyiaSenderListener listener, IBinder binder) {
            mListener = listener;
            mRemote = binder;
        }

        @Override
        public void binderDied() {
            BoyiaLog.d(TAG, "linkToDeath callback");
            mRemote.unlinkToDeath(this, 0);
            mListener.onSenderEnd();
        }
    }
}
