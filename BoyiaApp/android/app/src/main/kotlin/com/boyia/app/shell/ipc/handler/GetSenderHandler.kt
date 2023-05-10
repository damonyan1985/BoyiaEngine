package com.boyia.app.shell.ipc.handler

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaIpcSender
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.core.launch.BoyiaAppLauncher
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule

class GetSenderHandler(private val module: IPCModule): IBoyiaIPCHandler {
    companion object {
        const val TAG = "GetSenderHandler"
    }

    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        val aid = bundle?.getInt(ApiConstants.ApiKeys.BINDER_AID)
        val binder = bundle?.getBinder(ApiConstants.ApiNames.SEND_BINDER);
        val sender = IBoyiaIpcSender.BoyiaSenderProxy(binder)

        BoyiaLog.d(TAG, "GetSenderHandler handle aid=$aid and binder=$binder")
        /**
         * 如果子进程挂了，则清除
         */
        sender.whileSenderEnd {
            BoyiaLog.d(TAG, "boyia app exit and appid is: $aid")
            if (aid != null) {
                module.removeSender(aid)
                BoyiaAppLauncher.launcher().notifyAppExit(aid)
            }
        }

        if (aid != null) {
            module.registerSender(aid, sender)
        }
        cb.callback(null)
    }
}