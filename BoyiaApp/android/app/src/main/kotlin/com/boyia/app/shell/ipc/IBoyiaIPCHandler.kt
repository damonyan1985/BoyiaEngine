package com.boyia.app.shell.ipc

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback

/**
 * ipc调用会在binder线程中调用
 */
interface IBoyiaIPCHandler {
    fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback)
}