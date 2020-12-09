package com.boyia.app.common.ipc;

public interface IBoyiaIpcCallback {
    void callback(BoyiaIpcData message);
    IpcScheduler scheduler();

    interface IpcScheduler {
        void run(Runnable runnable);
    }
}
