package com.boyia.app.shell.ipc

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaIpcSender

class BoyiaHostBinder : IBoyiaIpcSender.BoyiaSenderStub() {
    override fun sendMessageSync(data: BoyiaIpcData?): BoyiaIpcData? {
        return null
    }

    override fun sendMessageAsync(data: BoyiaIpcData?, callback: IBoyiaIpcCallback?) {
    }
}