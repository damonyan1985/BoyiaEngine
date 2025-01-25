package com.boyia.app.shell.ipc.handler

import android.os.Bundle
import com.boyia.app.common.ipc.BoyiaIpcData
import com.boyia.app.common.ipc.IBoyiaIpcCallback
import com.boyia.app.common.ipc.IBoyiaSender
import com.boyia.app.core.api.ApiConstants.ApiKeys
import com.boyia.app.shell.ipc.IBoyiaIPCHandler
import com.boyia.app.shell.module.IPCModule
import com.boyia.app.shell.update.Downloader
import com.boyia.app.shell.update.Downloader.DownLoadProgressListener

/**
 * 处理下载接口
 */
class DownloadHandler(private val ipcModule: IPCModule): IBoyiaIPCHandler, DownLoadProgressListener {
    private var aid: Int = 0
    private var callbackId: Long = 0L
    private var sender: IBoyiaSender? = null
    override fun handle(data: BoyiaIpcData?, cb: IBoyiaIpcCallback) {
        val bundle = data?.params
        bundle?.getInt(ApiKeys.BINDER_AID)?.let {
            aid = it
            sender = ipcModule.appSender(aid)
        }
        bundle?.getLong(ApiKeys.BINDER_AID)?.let {
            callbackId = it
        }
        val url = bundle?.getString(ApiKeys.REQUEST_URL)
        Downloader(this).download(url)
    }

    override fun onProgress(current: Long, size: Long) {
        if (aid != 0 && callbackId != 0L) {
            val bundle = Bundle()
            bundle.putLong(ApiKeys.CALLBACK_ID, callbackId)
            sender?.sendMessageSync(BoyiaIpcData())
        }
    }

    override fun onCompleted() {
    }
}