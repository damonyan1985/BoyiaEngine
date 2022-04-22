package com.boyia.app.shell.ipc

import android.os.IBinder
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IPCModule

class BoyiaIPCModule : IPCModule {
    private var binder: BoyiaHostBinder? = null
    override fun getBinder(): IBinder? {
        return binder
    }

    override fun init() {
        binder = BoyiaHostBinder()
    }

    override fun show(context: IModuleContext) {
    }

    override fun dispose() {
    }
}