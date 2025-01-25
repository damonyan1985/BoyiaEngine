package com.boyia.app.shell.login

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.json.BoyiaJson
import com.boyia.app.common.utils.BoyiaLog
import com.boyia.app.core.api.ApiConstants
import com.boyia.app.loader.job.JobScheduler
import com.boyia.app.shell.BoyiaShellActivity
import com.boyia.app.shell.model.BoyiaLoginInfo
import com.boyia.app.shell.model.BoyiaUserData
import com.boyia.app.shell.model.BoyiaUserInfo
import com.boyia.app.shell.module.ILoginModule
import com.boyia.app.shell.module.IModuleContext
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.module.LoginListener
import com.boyia.app.shell.module.ModuleManager
import com.boyia.app.shell.util.PermissionCallback

/**
 * 提供给boyia app调用
 */
class LoginActivity: BoyiaShellActivity(), IModuleContext {
    companion object {
        const val TAG = "LoginActivity"
    }

    override fun onCreate(bundle: Bundle?) {
        super.onCreate(bundle)

        val ipcModule = ModuleManager.instance().getModule(ModuleManager.IPC) as IPCModule
        val aid = intent.getIntExtra(ApiConstants.ApiKeys.BINDER_AID, 0)
        val callbackId = intent.getLongExtra(ApiConstants.ApiKeys.CALLBACK_ID, 0)
        if (aid == 0) {
            return
        }
        val sender = ipcModule.appSender(aid)

        val loginModule = ModuleManager.instance().getModule(ModuleManager.LOGIN) as ILoginModule
        loginModule.addLoginListener(object: LoginListener {
            override fun onLogined(info: BoyiaUserInfo) {
                // 使用sender回传给boyia app
                val userData = BoyiaUserData()
                userData.data = BoyiaLoginInfo.instance().user;
                userData.userToken = BoyiaLoginInfo.instance().token;

                val json = BoyiaJson.toJson(userData)
                val bundle = Bundle()
                bundle.putString(ApiConstants.ApiNames.USER_LOGIN, json)
                bundle.putLong(ApiConstants.ApiKeys.CALLBACK_ID, callbackId)

                sender?.sendMessageAsync(BoyiaIpcData(ApiConstants.ApiNames.USER_LOGIN, bundle), object: IBoyiaIpcCallback {
                    override fun callback(message: BoyiaIpcData?) {
                        BoyiaLog.d(TAG, "");
                    }

                    override fun scheduler(): IBoyiaIpcCallback.IpcScheduler {
                        return IBoyiaIpcCallback.IpcScheduler { runnable -> JobScheduler.jobScheduler().sendJob { runnable?.run() } }
                    }
                })
            }
        })
        loginModule.show(this)
    }

    override fun getActivity(): AppCompatActivity {
        return this
    }
}