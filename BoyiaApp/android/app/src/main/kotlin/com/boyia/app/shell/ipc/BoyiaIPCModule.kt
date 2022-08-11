package com.boyia.app.shell.ipc

import android.os.IBinder
import com.boyia.app.common.ipc.IBoyiaSender
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.shell.ipc.handler.*
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IPCModule
import java.lang.ref.WeakReference
import java.lang.reflect.Constructor
import java.util.concurrent.ConcurrentHashMap

class BoyiaIPCModule : IPCModule {
    private var binder: BoyiaHostBinder? = null
    private var context: WeakReference<IModuleContext>? = null
    private lateinit var handlerMap: ConcurrentHashMap<String?, IBoyiaHandlerCreator>
    private lateinit var appSenderMap: ConcurrentHashMap<Int, IBoyiaSender>

    override fun getBinder(): IBinder? {
        return binder
    }

    override fun appSender(aid: Int): IBoyiaSender? {
        return appSenderMap[aid]
    }

    override fun init() {
        binder = BoyiaHostBinder(this)
        handlerMap = ConcurrentHashMap<String?, IBoyiaHandlerCreator>()
        appSenderMap = ConcurrentHashMap<Int, IBoyiaSender>()
        register(ApiConstants.ApiNames.LOCAL_SHARE_SET, SetShareHandler::class.java)
        register(ApiConstants.ApiNames.LOCAL_SHARE_GET, GetShareHandler::class.java)
        register(ApiConstants.ApiNames.NOTIFICATION_NAME, SendNotificationHandler::class.java)
        register(ApiConstants.ApiNames.USER_INFO, GetUserInfoHandler::class.java)
        register(ApiConstants.ApiNames.USER_LOGIN, LoginHandler::class.java)
        register(ApiConstants.ApiNames.SEND_BINDER, GetSenderHandler::class.java, true)
    }

    override fun registerSender(aid: Int, sender: IBoyiaSender) {
        appSenderMap[aid] = sender
    }

    /**
     * 内部包一个handler对象生成器，获取handler时通过生成器生成
     */
    private fun register(method: String, type: Class<out IBoyiaIPCHandler>, async: Boolean = false) {
        handlerMap[method] = object : IBoyiaHandlerCreator {
            override fun create(): IBoyiaIPCHandler {
                if (async) {
                    return type.getConstructor(IPCModule::class.java).newInstance(this)
                }

                return type.newInstance();
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

    override fun hide() {
    }

    override fun dispose() {
    }

    interface IBoyiaHandlerCreator {
        fun create(): IBoyiaIPCHandler
    }
}