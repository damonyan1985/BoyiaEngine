package com.boyia.app.shell.ipc.handler

import android.os.Bundle
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaSender
import com.boyia.app.core.api.ApiConstants.ApiKeys
import com.boyia.app.core.api.ApiConstants.ApiNames
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.update.Downloader
import com.boyia.app.shell.update.Downloader.DownLoadProgressListener
import org.json.JSONObject

/**
 * 处理下载接口
 */
class DownloadHandler(private val ipcModule: IPCModule): IBoyiaIPCHandler, DownLoadProgressListener {
    private var aid: Int = 0
    private var callbackID: Long = 0L
    private var sender: IBoyiaSender? = null

    /**
     * 处理boyia引擎download接口
     */
    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        bundle?.getInt(ApiKeys.BINDER_AID)?.let {
            aid = it
            sender = ipcModule.appSender(aid)
        }
        bundle?.getLong(ApiKeys.BINDER_AID)?.let {
            callbackID = it
        }
        val url = bundle?.getString(ApiKeys.REQUEST_URL)
        Downloader(this).download(url)
    }

    /**
     * 回调给boyia引擎执行
     */
    override fun onProgress(current: Long, size: Long) {
        if (aid != 0 && callbackID != 0L) {
            val args = JSONObject()
                .put("current", current)
                .put("size", size)
            val bundle = Bundle()
            bundle.putLong(ApiKeys.CALLBACK_ID, callbackID)
            bundle.putString(ApiKeys.CALLBACK_ARGS, args.toString())
            sender?.sendMessageSync(BoyiaIpcData(ApiNames.DOWNLOAD, bundle))
        }
    }

    override fun onCompleted() {
        if (aid != 0 && callbackID != 0L) {
            val args = JSONObject().put("completed", true)
            val bundle = Bundle()
            bundle.putLong(ApiKeys.CALLBACK_ID, callbackID)
            bundle.putString(ApiKeys.CALLBACK_ARGS, args.toString())
            sender?.sendMessageSync(BoyiaIpcData(ApiNames.DOWNLOAD, bundle))
        }
    }
}