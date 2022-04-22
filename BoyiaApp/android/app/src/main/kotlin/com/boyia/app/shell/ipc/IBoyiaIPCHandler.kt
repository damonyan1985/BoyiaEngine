package com.boyia.app.shell.ipc

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback

interface IBoyiaIPCHandler {
    fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback);
}