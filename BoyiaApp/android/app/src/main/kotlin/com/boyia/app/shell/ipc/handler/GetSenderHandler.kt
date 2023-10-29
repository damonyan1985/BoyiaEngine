package com.boyia.app.shell.ipc.handler

import android.os.Bundle
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaIpcSender
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.core.launch.BoyiaAppLauncher
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.permission.BoyiaDevicePermission

/**
 * 接受boyia app的binder
 * 同时返回权限实现类
 */
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
         * 此处接受子进程死亡消息
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

        /**
         * 因boyia core中并未使用androidx，所以需要将类名传入，提供给boyia子进程去实例化权限类
         */
        val result = Bundle()
        result.putString(data?.method, BoyiaDevicePermission::class.java.name)
        cb.callback(BoyiaIpcData(data?.method, result))
    }
}