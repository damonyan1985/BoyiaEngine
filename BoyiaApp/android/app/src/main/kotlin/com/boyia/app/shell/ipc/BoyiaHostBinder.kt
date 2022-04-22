package com.boyia.app.shell.ipc

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaIpcSender
import com.boyia.app.loader.job.JobScheduler

class BoyiaHostBinder(private val module: BoyiaIPCModule) : IBoyiaIpcSender.BoyiaSenderStub() {

    override fun sendMessageSync(data: BoyiaIpcData?): BoyiaIpcData? {
        return null
    }

    override fun sendMessageAsync(data: BoyiaIpcData?, callback: IBoyiaIpcCallback) {
        JobScheduler.jobScheduler().sendJob {
            val handler = module.getHandler(data?.method) ?: return@sendJob
            handler.handle(data, callback)
        }
    }
}