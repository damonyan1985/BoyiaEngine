package com.boyia.app.shell.ipc.handler

import android.content.Intent
import com.boyia.app.common.BaseApplication
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.login.LoginActivity
import com.boyia.app.shell.module.IPCModule

class LoginHandler: IBoyiaIPCHandler {
    /**
     * 使用aid启动登录页面，告诉登录页面是哪个boyia app需要获取登录信息
     */
    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params

        val intent = Intent()
        intent.action = "com.boyia.app.shell.login.action"
        // 使用application的context来启动activity时必须加入FLAG_ACTIVITY_NEW_TASK
        intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
        intent.putExtra(ApiConstants.ApiKeys.BINDER_AID, bundle?.getInt(ApiConstants.ApiKeys.BINDER_AID))
        intent.putExtra(ApiConstants.ApiKeys.CALLBACK_ID, bundle?.getLong(ApiConstants.ApiKeys.CALLBACK_ID))
        BaseApplication.getInstance().startActivity(intent)
        cb.callback(null)
    }
}