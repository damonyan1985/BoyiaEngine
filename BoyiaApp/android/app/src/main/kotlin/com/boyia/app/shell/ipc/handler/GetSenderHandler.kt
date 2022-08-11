package com.boyia.app.shell.ipc.handler

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaIpcSender
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule

class GetSenderHandler(private val module: IPCModule): IBoyiaIPCHandler {
    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        val aid = bundle?.getInt(ApiConstants.ApiKeys.BINDER_AID)
        val sender = IBoyiaIpcSender.BoyiaSenderStub.asInterface(
                bundle?.getBinder(ApiConstants.ApiNames.SEND_BINDER))

        if (aid != null) {
            module.registerSender(aid, sender)
        }
        cb.callback(null)
    }
}