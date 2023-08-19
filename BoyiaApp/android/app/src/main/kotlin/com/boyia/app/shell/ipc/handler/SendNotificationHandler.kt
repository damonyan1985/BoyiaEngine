package com.boyia.app.shell.ipc.handler

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.core.api.ApiConstants.ApiKeys
import com.boyia.app.core.launch.BoyiaAppInfo
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.util.CommonFeatures

class SendNotificationHandler(private val module: IPCModule): IBoyiaIPCHandler {
    companion object {
        const val TAG = "SendNotificationHandler"
    }

    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        // kotlin必须设置classloader，否则getParcelable时无法找到实体类
        bundle?.classLoader = BoyiaAppInfo::class.java.classLoader
        val appInfo = bundle?.getParcelable<BoyiaAppInfo>(ApiKeys.NOTIFICATION_APP_INFO) as BoyiaAppInfo
        val msg = bundle?.getString(ApiKeys.NOTIFICATION_MSG)
        val action = bundle?.getString(ApiKeys.NOTIFICATION_ACTION)

        if (BoyiaUtils.isTextEmpty(msg)
                || BoyiaUtils.isTextEmpty(appInfo?.mAppCover)
                || BoyiaUtils.isTextEmpty(action)) {
            cb.callback(null)
            return
        }

        BoyiaLog.d(TAG, "title = ${appInfo?.mAppName}, icon = ${appInfo?.mAppCover}, action = $action msg=$msg")

        module.sendNotification {
            CommonFeatures.sendNotification(action!!, appInfo!!.mAppName, appInfo.mAppCover, msg!!)
        }

        cb.callback(null)
    }
}