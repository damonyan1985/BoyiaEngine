package com.boyia.app.shell.ipc.handler

import android.os.Bundle
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaShare
import com.boyia.app.core.api.ApiConstants.ApiKeys
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule

class GetShareHandler: IBoyiaIPCHandler {
    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        val key = bundle?.get(ApiKeys.IPC_SHARE_KEY)

        if (key is String) {
            val value = BoyiaShare.get(key, null)
            BoyiaLog.d(SetShareHandler.TAG, "SetShareHandler handle key = $key and value = $value")

            val bundle = Bundle()
            putValue(bundle, value)
            cb.callback(BoyiaIpcData(data.method, bundle))
        } else {
            cb.callback(null)
        }
    }

    private fun putValue(bundle: Bundle, value: Any) {
        when (value) {
            is String -> bundle.putString(ApiKeys.IPC_SHARE_KEY, value)
            is Int -> bundle.putInt(ApiKeys.IPC_SHARE_KEY, value)
            is Boolean -> bundle.putBoolean(ApiKeys.IPC_SHARE_KEY, value)
            is Long -> bundle.putLong(ApiKeys.IPC_SHARE_KEY, value)
            is Float -> bundle.putFloat(ApiKeys.IPC_SHARE_KEY, value)
        }
    }
}