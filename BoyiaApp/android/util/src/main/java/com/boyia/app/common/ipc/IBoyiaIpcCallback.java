package com.boyia.app.common.ipc;

/**
 * IPC回调
 */
public interface IBoyiaIpcCallback {
    /**
     * 回调消息
     * @param message
     */
    void callback(BoyiaIpcData message);

    /**
     * 设置ipc接口运行线程
     * @return IpcScheduler
     */
    IpcScheduler scheduler();

    interface IpcScheduler {
        void run(Runnable runnable);
    }
}
