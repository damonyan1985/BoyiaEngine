package com.boyia.app.shell.ipc.handler

import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.core.api.ApiConstants.ApiKeys
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.util.CommonFeatures

class SendNotificationHandler: IBoyiaIPCHandler {
    companion object {
        const val TAG = "SendNotificationHandler"
    }

    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        val title = bundle?.getString(ApiKeys.NOTIFICATION_TITLE)
        val icon = bundle?.getString(ApiKeys.NOTIFICATION_ICON)
        val action = bundle?.getString(ApiKeys.NOTIFICATION_ACTION)

        if (BoyiaUtils.isTextEmpty(title)
                || BoyiaUtils.isTextEmpty(icon)
                || BoyiaUtils.isTextEmpty(action)) {
            cb.callback(null)
            return
        }

        BoyiaLog.d(TAG, "title = $title, icon = $icon, action = $action")

        CommonFeatures.sendNotification(action!!, title!!, icon!!)
        cb.callback(null)
    }
}