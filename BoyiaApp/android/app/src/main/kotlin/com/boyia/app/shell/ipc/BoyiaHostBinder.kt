package com.boyia.app.shell.ipc

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaIpcSender
import com.boyia.app.loader.job.JobScheduler

/**
 * shell工程binder
 */
class BoyiaHostBinder(private val module: BoyiaIPCModule) : IBoyiaIpcSender.BoyiaSenderStub() {

    override fun sendMessageSync(data: BoyiaIpcData?): BoyiaIpcData? {
        return null
    }

    override fun sendMessageAsync(data: BoyiaIpcData?, callback: IBoyiaIpcCallback) {
        val handler = module.getHandler(data?.method) ?: return
        handler.handle(data, callback)
    }
}