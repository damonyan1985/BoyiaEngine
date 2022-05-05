package com.boyia.app.shell.ipc.handler

import android.os.Bundle
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaShare
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.BoyiaConstants.IPCNameConstants

class GetShareHandler: IBoyiaIPCHandler {
    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        val key = bundle?.get(IPCNameConstants.IPC_SHARE_KEY)

        if (key is String) {
            val value = BoyiaShare.get(key, null)
            BoyiaLog.d(SetShareHandler.TAG, "SetShareHandler handle key = $key and value = $value")

            val bundle = Bundle();
            putValue(bundle, value)
            cb.callback(BoyiaIpcData(data.method, bundle))
        }
    }

    private fun putValue(bundle: Bundle, value: Any) {
        when (value) {
            is String -> bundle.putString(IPCNameConstants.IPC_SHARE_KEY, value)
            is Int -> bundle.putInt(IPCNameConstants.IPC_SHARE_KEY, value)
            is Boolean -> bundle.putBoolean(IPCNameConstants.IPC_SHARE_KEY, value)
            is Long -> bundle.putLong(IPCNameConstants.IPC_SHARE_KEY, value)
            is Float -> bundle.putFloat(IPCNameConstants.IPC_SHARE_KEY, value)
        }
    }
}