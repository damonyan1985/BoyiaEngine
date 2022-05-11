package com.boyia.app.shell.ipc

import android.os.IBinder
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.BoyiaConstants.IPCNameConstants;
import com.boyia.app.shell.ipc.handler.GetShareHandler
import com.boyia.app.shell.ipc.handler.SetShareHandler
import java.lang.ref.WeakReference
import java.util.concurrent.ConcurrentHashMap

class BoyiaIPCModule : IPCModule {
    private var binder: BoyiaHostBinder? = null
    private var context: WeakReference<IModuleContext>? = null
    private lateinit var handlerMap: ConcurrentHashMap<String?, IBoyiaHandlerCreator>

    override fun getBinder(): IBinder? {
        return binder
    }

    override fun init() {
        binder = BoyiaHostBinder(this)
        handlerMap = ConcurrentHashMap<String?, IBoyiaHandlerCreator>()
        register(IPCNameConstants.LOCAL_SHARE_SET, SetShareHandler::class.java)
        register(IPCNameConstants.LOCAL_SHARE_GET, GetShareHandler::class.java)
    }

    /**
     * 内部包一个handler对象生成器，获取handler时通过生成器生成
     */
    private fun register(method: String, type: Class<out IBoyiaIPCHandler>) {
        handlerMap[method] = object : IBoyiaHandlerCreator {
            override fun create(): IBoyiaIPCHandler {
                return type.newInstance()
            }
        }
    }

    fun getHandler(key: String?): IBoyiaIPCHandler? {
        if (BoyiaUtils.isTextEmpty(key)) {
            return null
        }

        return handlerMap[key]?.create()
    }

    override fun show(ctx: IModuleContext) {
        // 保存context，用于handler调用时处理activity相关的操作
        context = WeakReference(ctx)
    }

    override fun dispose() {
    }

    interface IBoyiaHandlerCreator {
        fun create(): IBoyiaIPCHandler
    }
}