package com.boyia.app.shell.ipc.handler

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaShare
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.BoyiaConstants.IPCNameConstants

class SetShareHandler: IBoyiaIPCHandler {
    companion object {
        const val TAG = "SetShareHandler"
    }

    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        val key = bundle?.get(IPCNameConstants.IPC_SHARE_KEY)
        val value = bundle?.get(IPCNameConstants.IPC_SHARE_VALUE)

        BoyiaLog.d(TAG, "SetShareHandler handle key = $key and value = $value")
        if (key is String) {
            BoyiaShare.set(key, value)
        }

        cb.callback(null)
    }
}