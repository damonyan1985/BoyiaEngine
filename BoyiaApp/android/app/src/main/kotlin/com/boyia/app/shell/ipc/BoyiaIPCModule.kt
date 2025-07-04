package com.boyia.app.shell.ipc

import android.os.IBinder
import com.boyia.app.common.ipc.IBoyiaSender
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.common.utils.BoyiaUtils
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.shell.ipc.handler.*
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.util.PermissionCallback
import java.lang.ref.WeakReference
import java.lang.reflect.Constructor
import java.util.concurrent.ConcurrentHashMap

class BoyiaIPCModule : IPCModule {
    companion object {
        const val TAG = "BoyiaIPCModule"
    }

    private var binder: BoyiaHostBinder? = null
    private var context: WeakReference<IModuleContext>? = null
    private lateinit var handlerMap: ConcurrentHashMap<String?, IBoyiaHandlerCreator>
    // 简版AMS
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
        register(ApiConstants.ApiNames.DOWNLOAD, DownloadHandler::class.java)
        register(ApiConstants.ApiNames.NOTIFICATION_NAME, SendNotificationHandler::class.java, true)
        register(ApiConstants.ApiNames.USER_INFO, GetUserInfoHandler::class.java)
        register(ApiConstants.ApiNames.USER_LOGIN, LoginHandler::class.java, true)
        register(ApiConstants.ApiNames.SEND_BINDER, GetSenderHandler::class.java, true)
    }

    override fun registerSender(aid: Int, sender: IBoyiaSender) {
        appSenderMap[aid] = sender
    }

    override fun removeSender(aid: Int) {
        appSenderMap.remove(aid);
    }

    override fun sendNotification(permissionCallback: PermissionCallback) {
        context?.get()?.let {
            it.sendNotification {
                permissionCallback()
            }
        }
    }

    /**
     * 内部包一个handler对象生成器，获取handler时通过生成器生成
     */
    private fun register(method: String, type: Class<out IBoyiaIPCHandler>, async: Boolean = false) {
        handlerMap[method] = object : IBoyiaHandlerCreator {
            override fun create(): IBoyiaIPCHandler {
                if (async) {
                    val handler = type.getConstructor(IPCModule::class.java).newInstance(this@BoyiaIPCModule)
                    BoyiaLog.d(TAG, "BoyiaIPCModule register handler=$handler")
                    return handler
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