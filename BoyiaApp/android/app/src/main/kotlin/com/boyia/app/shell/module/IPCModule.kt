package com.boyia.app.shell.module

import android.os.IBinder
import com.boyia.app.common.ipc.IBoyiaSender
import com.boyia.app.shell.util.PermissionCallback

interface IPCModule : IModule {
    fun getBinder(): IBinder?

    /**
     * boyia app注册的sender
     */
    fun appSender(aid: Int): IBoyiaSender?

    /**
     * 注册sender
     */
    fun registerSender(aid: Int, sender: IBoyiaSender)

    /**
     * 删除sender
     */
    fun removeSender(aid: Int)

    /**
     * 发送通知
     */
    fun sendNotification(permissionCallback: PermissionCallback)
}