package com.boyia.app.shell.ipc

import android.os.IBinder
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IPCModule
import java.util.concurrent.ConcurrentHashMap

class BoyiaIPCModule : IPCModule {
    private var binder: BoyiaHostBinder? = null
    private var handlerMap: Map<String?, IBoyiaIPCHandler>? = null

    override fun getBinder(): IBinder? {
        return binder
    }

    override fun init() {
        binder = BoyiaHostBinder(this)
        handlerMap = ConcurrentHashMap<String?, IBoyiaIPCHandler>()
    }

    fun getHandler(key: String?): IBoyiaIPCHandler? {
        if (BoyiaUtils.isTextEmpty(key)) {
            return null
        }
        return handlerMap?.get(key)
    }

    override fun show(context: IModuleContext) {
    }

    override fun dispose() {
    }
}