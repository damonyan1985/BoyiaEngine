package com.boyia.app.shell.ipc.handler

import android.os.Bundle
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.json.BoyiaJson
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.model.BoyiaLoginInfo
import com.boyia.app.shell.model.BoyiaUserData
import com.boyia.app.shell.module.IPCModule

// 获取本地用户信息
class GetUserInfoHandler: IBoyiaIPCHandler {
    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val userData = BoyiaUserData()
        userData.data = BoyiaLoginInfo.instance().user;
        userData.userToken = BoyiaLoginInfo.instance().token;

        val json = BoyiaJson.toJson(userData)
        val bundle = Bundle()
        bundle.putString(data?.method, json)
        cb.callback(BoyiaIpcData(data?.method, bundle))
    }
}